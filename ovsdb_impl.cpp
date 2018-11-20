/* 
 * File:   ovsdb_impl.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net *
 * 
 * Created on November 14, 2018, 5:04 PM
 */

// https://tools.ietf.org/html/rfc7047
// http://www.openvswitch.org//ovs-vswitchd.conf.db.5.pdf
// https://relaxdiego.com/2014/09/ovsdb.html
// https://www.jsonrpc.org/specification_v1

// json docs:  https://github.com/nlohmann/json

// TODO: connect via netlink to pull out raw interfaces and ports,
//    then can use interface to add into and remove from ovs

// TODO: use the 'set' ability to set mode for controller, rather than doing from command line

// TODO: use std::for_each structure for processing rather than the for loop method.
// TODO; use references when sub-dividing the json structures?

// TODO: convert other iterators over to std::for_each

// TOOD: consider scanning inbound strings with boost::spirit

// ovs-vsctl del-fail-mode ovsbr0
// ovs-vsctl set-fail-mode ovsbr0 secure

#include <iostream>
#include <algorithm>

#include <boost/asio/write.hpp>

#include "ovsdb_impl.h"

ovsdb_impl::ovsdb_impl( ovsdb& ovsdb_, asio::io_context& io_context )
:
  m_ep( "/var/run/openvswitch/db.sock" ),
  //m_ep( ip::tcp::v4(), 6640 ),
  m_ovsdb( ovsdb_ ),
  m_socket( io_context ),
  m_state( start )
{
  assert( BOOST_ASIO_HAS_LOCAL_SOCKETS );
  m_socket.connect( m_ep );
  
  // TODO: on read, confirm that the database is available, ensures validity of further queries
  // to show some queries: '# ovs-vsctl -vjsonrpc show'
  // table values are '# ovsdb-client dump'
  
  do_read(); // start up socket read 

  m_state = listdb;
  send_list_dbs();
  
}

ovsdb_impl::~ovsdb_impl( ) {
}

void ovsdb_impl::send( const std::string& sCmd ) {
  try {
    asio::async_write( 
      m_socket, boost::asio::buffer( sCmd ), 
      [this](boost::system::error_code ec, std::size_t cntWritten ){
        if ( ec ) {
          std::cout << "<<< ovsdb write error: " << ec.message() << std::endl;
        }
        else {
          std::cout << "<<< ovsdb written: " << cntWritten << std::endl;
        }
      } );
  }
  catch ( std::exception& e ) {
    std::cout << "<<< ovsdb error: " << e.what() << std::endl;
  }
}

void ovsdb_impl::send_list_dbs() {
  json j = {
    { "method", "list_dbs" },
    { "params", json::array() },
    { "id", 1 }
  };
  //std::cout << "*** test output: " << j << std::endl;
  send( j.dump() );
}

void ovsdb_impl::send_monitor_bridges() {
  json colSwitch, colBridge;
  json keys = json::object();

  colSwitch["columns"] = { "bridges", "db_version", "ovs_version", "external_ids" };
  keys["Open_vSwitch"] = json::array( { colSwitch } );

  colBridge["columns"] = { "datapath_id", "fail_mode", "name", "ports", "stp_enable" };
  keys["Bridge"]       = json::array( { colBridge } );

  json j = {
    { "id", 2 },
    { "method", "monitor" },
    { "params", { "Open_vSwitch", json::array( { "bridge" } ), keys } }
  };
  send( j.dump() );
}

void ovsdb_impl::send_monitor_ports() {
  json colPort;
  json keys = json::object();

  colPort["columns"] = { "interfaces", "name", "tag", "trunks", "vlan_mode" };
  keys["Port"]       = json::array( { colPort } );

  json j = {
    { "id", 3 },
    { "method", "monitor" },
    { "params", { "Open_vSwitch", json::array( { "port" } ), keys } }
  };

  send( j.dump() );
}

void ovsdb_impl::send_monitor_interfaces() {
  json colInterface;
  json keys = json::object();

  colInterface["columns"] = { "admin_state", "link_state", "name", "ofport", "ifindex","mac_in_use", "type" };
  keys["Interface"]       = json::array( { colInterface } );

  json j = {
    { "id", 4 },
    { "method", "monitor" },
    { "params", { "Open_vSwitch", json::array( { "interface" } ), keys } }
  };

  send( j.dump() );
}

void ovsdb_impl::send_monitor_statistics() {
  json colInterface;
  json keys = json::object();

  colInterface["columns"] = { "statistics" };
  keys["Interface"]       = json::array( { colInterface } );

  json j = {
    { "id", 5 },
    { "method", "monitor" },
    { "params", { "Open_vSwitch", json::array( { "statistics" } ), keys } }
  };

  send( j.dump() );
}

bool ovsdb_impl::parse_listdb( json& j ) {
  bool bResult( false );
  //std::cout << "listdb entries: ";
  std::for_each( j.begin(), j.end(), [&bResult](auto& key) {
    //std::cout << *iter;
    if ( "Open_vSwitch" == key ) {
      bResult = true;
    }
  });
  //std::cout << std::endl;
  return bResult;
}

bool ovsdb_impl::parse_bridge( json& j ) {

  auto& ovs = j["Open_vSwitch"];
  //std::cout << j.dump(2) << std::endl;
  for ( json::iterator iterOvs = ovs.begin(); ovs.end() != iterOvs; iterOvs++ ) {
    
    uuid_t uuidSwitch( iterOvs.key() );
    mapSwitch_t::iterator iterSwitch = m_mapSwitch.find( uuidSwitch );
    if ( m_mapSwitch.end() == iterSwitch ) {
      iterSwitch = m_mapSwitch.insert( m_mapSwitch.begin(), mapSwitch_t::value_type( uuidSwitch, switch_t() ) );
      if ( nullptr != m_ovsdb.m_f.fSwitchAdd ) m_ovsdb.m_f.fSwitchAdd( uuidSwitch );
    }
    
    auto& values = iterOvs.value()["new"];
    
    switch_t& sw( iterSwitch->second );
    
    sw.db_version = values["db_version"];
    sw.ovs_version = values["ovs_version"];
    
    auto& external_ids = values["external_ids"];
    if ( external_ids.is_array() ) {
      for ( json::iterator iterId = external_ids.begin(); iterId != external_ids.end(); iterId++ ) {
        if ( "map" == *iterId ) {
          iterId++;
          auto& elements = *iterId;
          //size_t cnt = elements.size();
          //std::string hostname;
          for ( json::iterator iterElement = elements.begin(); iterElement != elements.end(); iterElement++ ) {
            if ( "hostname" == (*iterElement)[0] ) {
              sw.hostname = (*iterElement)[1];
            }
          }
        }
      }
    }
    
    if ( nullptr != m_ovsdb.m_f.fSwitchUpdate ) m_ovsdb.m_f.fSwitchUpdate( uuidSwitch, sw );

    // create a function as it the code is used in two different places
    auto fAddBridge = [this, &sw](json& j){
      for ( json::iterator iterBridgeJson = j.begin(); j.end() != iterBridgeJson; iterBridgeJson++ ) {
        assert( "uuid" == (*iterBridgeJson) );
        iterBridgeJson++;
        uuid_t uuidBridge( *iterBridgeJson );
        setBridge_t::iterator iterBridgeSet = sw.setBridge.find( uuidBridge );
        if ( sw.setBridge.end() == iterBridgeSet ) {
          mapBridge_t::iterator iterBridgeMap = m_mapBridge.find( uuidBridge );
          assert( m_mapBridge.end() == iterBridgeMap );
          iterBridgeMap = m_mapBridge.insert( m_mapBridge.begin(), mapBridge_t::value_type( uuidBridge, bridge_t() ) );
          iterBridgeSet = sw.setBridge.insert( sw.setBridge.begin(), setBridge_t::value_type( uuidBridge ) );
          if ( nullptr != m_ovsdb.m_f.fBridgeAdd ) m_ovsdb.m_f.fBridgeAdd( uuidBridge );
        }
      }
    };

    auto& bridges = values["bridges"];
    //size_t cntBridges = bridges.size();
    for ( json::iterator iterBridge = bridges.begin(); bridges.end() != iterBridge; iterBridge++ ) {
      if ( (*iterBridge).is_array() ) {
        for ( json::iterator iterElements = (*iterBridge).begin(); (*iterBridge).end() != iterElements; iterElements++ ) {
          fAddBridge( *iterElements );
        }
      }
      else {
        fAddBridge( bridges );
      }
    }
  } // for iterOvs

  // --
  auto& bridge = j["Bridge"];
  for ( json::iterator iterBridge = bridge.begin(); bridge.end() != iterBridge; iterBridge++ ) {
    uuid_t uuidBridge( iterBridge.key() );
    bridge_t& br( m_mapBridge[ uuidBridge ] );  // assumes already exists
    auto& values = iterBridge.value()[ "new" ];
    //std::cout << values.dump(2) << std::endl;
    br.datapath_id = values[ "datapath_id" ];
    br.fail_mode = values[ "fail_mode" ];
    br.name = values[ "name" ];
    br.stp_enable = values[ "stp_enable" ];

    auto& ports = values[ "ports" ];
    for ( json::iterator iterElement = ports.begin(); ports.end() != iterElement; iterElement++ ) {
      if ( "set" == (*iterElement) ) {
        iterElement++;
        assert( ports.end() != iterElement );
        auto& set = *iterElement;
        for ( json::iterator iterPair = set.begin(); set.end() != iterPair; iterPair++ ) {
          auto& pair = *iterPair;
          for ( json::iterator iterPort = pair.begin(); pair.end() != iterPort; iterPort++ ) {
            assert( "uuid" == (*iterPort) );
            iterPort++;
            uuid_t uuidPort( *iterPort );
            m_mapPort.insert( mapPort_t::value_type( uuidPort, port_t() ) );
            br.setPort.insert( setPort_t::value_type( uuidPort ) );
            if ( nullptr != m_ovsdb.m_f.fPortAdd ) m_ovsdb.m_f.fPortAdd( uuidPort );
          }
        };
      }
    }
    
    if ( nullptr != m_ovsdb.m_f.fBridgeUpdate) m_ovsdb.m_f.fBridgeUpdate( uuidBridge, br );
    
  }
  
  return true;
}

bool ovsdb_impl::parse_port( json& j ) {

  auto& ports = j[ "Port" ];
  for ( json::iterator iterPortObject = ports.begin(); ports.end() != iterPortObject; iterPortObject++ ) {
    uuid_t uuidPort = iterPortObject.key();
    auto iterPort = m_mapPort.find( uuidPort );
    assert ( m_mapPort.end() != iterPort );
    auto& port( iterPort->second );
    auto& values = iterPortObject.value()[ "new" ];
    port.name = values[ "name" ];
    if ( values[ "tag" ].is_number() ) {
      port.tag = values[ "tag" ];
    }

    auto& trunks = values[ "trunks" ];
    for ( json::iterator iterElement = trunks.begin(); trunks.end() != iterElement; iterElement++ ) {
      assert( "set" == *iterElement );
      iterElement++;
      assert( trunks.end() != iterElement );
      assert( (*iterElement).is_array() );
      auto& vlans = *iterElement;
      for ( json::iterator iterVlan = vlans.begin(); vlans.end() != iterVlan; iterVlan++ ) {
        port.setTrunk.insert( std::set<uint16_t>::value_type( *iterVlan ) );
      }
    }

    if ( nullptr != m_ovsdb.m_f.fPortUpdate ) {
      m_ovsdb.m_f.fPortUpdate( uuidPort, port );
    }

    auto& interfaces = values[ "interfaces" ];
    for ( json::iterator iterInterface = interfaces.begin(); interfaces.end() != iterInterface; iterInterface++ ) {
      if ( (*iterInterface).is_array() ) {
        auto& elements = *iterInterface;
        assert( 0 );  // need to fix this with bonding and such (use function lamda as above)
        for ( json::iterator iterElement = elements.begin(); elements.end() != iterElement; iterElement++ ) {
        }
      }
      else {
        assert( "uuid" == *iterInterface );
        iterInterface++;
        assert( (*iterInterface).is_string() );
        uuid_t uuidInterface( *iterInterface );
        m_mapInterface.insert( mapInterface_t::value_type( uuidInterface, interface_t() ) );
        port.setInterface.insert( setInterface_t::value_type( uuidInterface ) );
        if ( nullptr != m_ovsdb.m_f.fInterfaceAdd ) m_ovsdb.m_f.fInterfaceAdd( uuidInterface );
      }
    }

  } // for iterPortObject
  

  return true;
}

bool ovsdb_impl::parse_interface( json& j ) {

  auto& interfaces = j["Interface"];

  for ( json::iterator iterInterfaceJson = interfaces.begin(); interfaces.end() != iterInterfaceJson; iterInterfaceJson++ ) {
    uuid_t uuidInterface = iterInterfaceJson.key();
    mapInterface_t::iterator iterInterface = m_mapInterface.find( uuidInterface );
    assert( m_mapInterface.end() != iterInterface );

    // TODO: use boost::spirit to decode the json values into this structure
    auto& interfaceJson = iterInterfaceJson.value()[ "new" ];
    auto& interfaceMap( iterInterface->second );
    interfaceMap.ifindex     =            interfaceJson[ "ifindex" ];
    interfaceMap.ofport      =            interfaceJson[ "ofport" ];
    interfaceMap.name        = std::move( interfaceJson[ "name" ] );
    interfaceMap.admin_state = std::move( interfaceJson[ "admin_state" ] );
    interfaceMap.link_state  = std::move( interfaceJson[ "link_state" ] );
    interfaceMap.mac_in_use  = std::move( interfaceJson[ "mac_in_use" ] );
    interfaceMap.ovs_type    = std::move( interfaceJson[ "type" ] );

    if ( nullptr != m_ovsdb.m_f.fInterfaceUpdate ) {
      m_ovsdb.m_f.fInterfaceUpdate( uuidInterface, interfaceMap );
    }

  }
  
  return true;
}

bool ovsdb_impl::parse_statistics( json& j ) {

  auto& interfaces = j["Interface"];

  for ( json::iterator iterInterfaceJson = interfaces.begin(); interfaces.end() != iterInterfaceJson; iterInterfaceJson++ ) {
    uuid_t uuidInterface = iterInterfaceJson.key();
    mapInterface_t::iterator iterInterface = m_mapInterface.find( uuidInterface );
    assert( m_mapInterface.end() != iterInterface );

    // TODO: use boost::spirit to decode the json values into this structure
    auto& interfaceJson = iterInterfaceJson.value()[ "new" ];
    auto& interfaceMap( iterInterface->second );

    // page 112 of openflow 1.4.1 spec shows how to get statistics via the controller channel
    //   therefore, this may go away at some point
    auto elements = interfaceJson[ "statistics" ];
    for ( json::iterator iterElements = elements.begin(); elements.end() != iterElements; iterElements++ ) {
      assert( "map" == *iterElements );
      iterElements++;
      assert( (*iterElements).is_array() );
      auto& statistics = *iterElements;
      for ( json::iterator iterCombo = statistics.begin(); statistics.end() != iterCombo; iterCombo++ ) {
        for ( json::iterator iterStatistic = (*iterCombo).begin(); (*iterCombo).end() != iterStatistic; iterStatistic++ ) {
          std::string name( *iterStatistic );
          iterStatistic++;
          interfaceMap.mapStatistics[ name ] = *iterStatistic;
        }
      }
    }
    
    if ( nullptr != m_ovsdb.m_f.fStatisticsUpdate ) {
      // need to convert before calling this function
      //m_ovsdb.m_f.fStatisticsUpdate( uuidInterface, mapStatistics );
    }
    
  }

  return true;
}

void ovsdb_impl::do_read() {
  m_vRx.resize( max_length );
  m_socket.async_read_some( boost::asio::buffer(m_vRx),
      [this](boost::system::error_code ec, const std::size_t lenRead)
      {
        if (ec) {
          std::cout << ">>> ovsdb read error: " << ec.message() << std::endl;
        }
        else {
          std::cout << ">>> ovsdb total read length: " << lenRead << std::endl;
          for ( size_t ix = 0; ix < lenRead; ix++ ) {
            std::cout << m_vRx[ ix ];
          }
          std::cout << std::endl;
          auto j = json::parse( m_vRx.begin(), m_vRx.begin() + lenRead );
          //std::cout << j.dump(2) << std::endl;
          std::cout << ">>> ovsdb read end." << std::endl;

          // process read state
          switch ( m_state ) {
            case start:
              std::cout << "*** something arrived in state: start, stuck here" << std::endl;
              break;
            case listdb: {
                m_state = stuck;

                assert( j["error"].is_null() );
                assert( 1 == j["id"] );

                auto& result = j["result"];
                if ( result.is_array() ) {
                  parse_listdb( result );
                  
                  m_state = startBridgeMonitor;
                  send_monitor_bridges();
                }
                else {
                  std::cout << "ovsdb stuck in listdb" << std::endl;
                }

              }
              break;
            case startBridgeMonitor: {
                m_state = stuck;

                assert( j["error"].is_null() );
                assert( 2 == j["id"] );

                auto& result = j["result"];
                parse_bridge( result );

                m_state = startPortMonitor;
                send_monitor_ports();

              }
              break;
            case startPortMonitor: {
                m_state = stuck;

                assert( j["error"].is_null() );
                assert( 3 == j["id"] );

                auto& result = j["result"];
                parse_port( result );

                m_state = startInterfaceMonitor;
                send_monitor_interfaces();
                
              }
              break;
            case startInterfaceMonitor: {
                m_state = stuck;
              
                assert( j["error"].is_null() );
                assert( 4 == j["id"] ); // will an update inter-leave here?  
                  // should we just do a big switch on in coming id's to be more flexible?
                  // then mark a vector of flags to indicate that it has been processed?
                
                auto& result = j["result"];
                parse_interface( result );

                m_state = startStatisticsMonitor;
                send_monitor_statistics();
                
              }
              break;
            case startStatisticsMonitor: {
                m_state = stuck;
              
                assert( j["error"].is_null() );
                assert( 5 == j["id"] ); // will an update inter-leave here?  
                  // should we just do a big switch on in coming id's to be more flexible?
                  // then mark a vector of flags to indicate that it has been processed?
                
                auto& result = j["result"];
                parse_statistics( result );

                m_state = listen;
              }
              break;
            case listen: {
                // process the monitor/update message
                // TODO: move into parse_update
                assert( j["id"].is_null() );
                assert( "update" == j["method"] );
                auto& params = j["params"];
                json::iterator iterParams = params.begin();
                assert( (*iterParams).is_array() );
                auto& list = *iterParams;
                iterParams++;
                assert( (*iterParams).is_object() );
                auto& items = *iterParams;
                iterParams++;
                assert( params.end() == iterParams );
                std::for_each( list.begin(), list.end(), [this, &items](auto& key) {
                  // use spirit to parse the strings?
                  if ( "bridge" == key ) {
                    parse_bridge( items );
                  }
                  if ( "port" == key ) {
                    parse_port( items );
                  }
                  if ( "interface" == key ) {
                    parse_interface( items );
                  }
                  if ( "statistics" == key ) {
                    parse_statistics( items );
                  }
                } );
              }
              break;
            case stuck:
              std::cout << "ovsdb arrived in stuck state" << std::endl;
              break;
          }
        }
        do_read();
      });
}