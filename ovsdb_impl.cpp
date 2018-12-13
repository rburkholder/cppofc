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

// TODO: http://boost-spirit.com/home/articles/qi-example/parsing-a-list-of-key-value-pairs-using-spirit-qi/

// ovs-vsctl del-fail-mode ovsbr0
// ovs-vsctl set-fail-mode ovsbr0 secure

#include <iostream>
#include <algorithm>

#include <boost/asio/write.hpp>

#include "ovsdb_impl.h"

namespace ovsdb {

decode_impl::decode_impl( decode& ovsdb_, asio::io_context& io_context )
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

decode_impl::~decode_impl( ) {
}

void decode_impl::send( const std::string& sCmd ) {
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

void decode_impl::send_list_dbs() {
  json j = {
    { "method", "list_dbs" },
    { "params", json::array() },
    { "id", 1 }
  };
  //std::cout << "*** test output: " << j << std::endl;
  send( j.dump() );
}

void decode_impl::send_monitor_bridges() {
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

void decode_impl::send_monitor_ports() {
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

void decode_impl::send_monitor_interfaces() {
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

void decode_impl::send_monitor_statistics() {
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

bool decode_impl::parse_listdb( const json& j ) {
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

bool decode_impl::parse_bridge( const json& j ) {

  //std::cout << j.dump(2) << std::endl;

  static const std::string sOpenvSwitch( "Open_vSwitch" ); // doesn't exist on interface add/removal
  if ( j.end() != j.find( sOpenvSwitch ) ) {
    auto& ovs = j["Open_vSwitch"];
    for ( json::const_iterator iterOvs = ovs.begin(); ovs.end() != iterOvs; iterOvs++ ) {

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
        for ( json::const_iterator iterId = external_ids.begin(); iterId != external_ids.end(); iterId++ ) {
          if ( "map" == *iterId ) {
            iterId++;
            auto& elements = *iterId;
            //size_t cnt = elements.size();
            //std::string hostname;
            for ( json::const_iterator iterElement = elements.begin(); iterElement != elements.end(); iterElement++ ) {
              if ( "hostname" == (*iterElement)[0] ) {
                sw.hostname = (*iterElement)[1];
              }
            }
          }
        }
      }

      if ( nullptr != m_ovsdb.m_f.fSwitchUpdate ) m_ovsdb.m_f.fSwitchUpdate( uuidSwitch, sw );

      // create a function as it the code is used in two different places
      auto fAddBridge = [this, &sw](const uuid_t& uuidSwitch, const json& j){
        for ( json::const_iterator iterBridgeJson = j.begin(); j.end() != iterBridgeJson; iterBridgeJson++ ) {
          assert( "uuid" == (*iterBridgeJson) );
          iterBridgeJson++;
          uuid_t uuidBridge( *iterBridgeJson );
          setBridge_t::iterator iterBridgeSet = sw.setBridge.find( uuidBridge );
          if ( sw.setBridge.end() == iterBridgeSet ) {
            mapBridge_t::iterator iterBridgeMap = m_mapBridge.find( uuidBridge );
            assert( m_mapBridge.end() == iterBridgeMap );
            iterBridgeMap = m_mapBridge.insert( m_mapBridge.begin(), mapBridge_t::value_type( uuidBridge, bridge_t() ) );
            iterBridgeSet = sw.setBridge.insert( sw.setBridge.begin(), setBridge_t::value_type( uuidBridge ) );
            if ( nullptr != m_ovsdb.m_f.fBridgeAdd ) m_ovsdb.m_f.fBridgeAdd( uuidSwitch, uuidBridge );
          }
        }
      };

      auto& bridges = values["bridges"];
      //size_t cntBridges = bridges.size();
      for ( json::const_iterator iterBridge = bridges.begin(); bridges.end() != iterBridge; iterBridge++ ) {
        if ( "uuid" == *iterBridge ) {
          fAddBridge( uuidSwitch, bridges );
        }
        if ( "set" == *iterBridge ) {
          iterBridge++;
          auto& list = *iterBridge;
          for ( json::const_iterator iterList = list.begin(); list.end() != iterList; iterList++ ) {
            fAddBridge( uuidSwitch, *iterList );
          }
        }
      }
    } // for iterOvs

  }

  // --

  auto fAddPort = [this](const uuid_t& uuidBridge, bridge_t& br, const json& j ){
    //auto& pair = *iterPair;
    for ( json::const_iterator iterPort = j.begin(); j.end() != iterPort; iterPort++ ) {
      assert( "uuid" == (*iterPort) );
      iterPort++;
      uuid_t uuidPort( *iterPort );
      m_mapPort.insert( mapPort_t::value_type( uuidPort, port_t() ) );
      br.setPort.insert( setPort_t::value_type( uuidPort ) );
      if ( nullptr != m_ovsdb.m_f.fPortAdd ) m_ovsdb.m_f.fPortAdd( uuidBridge, uuidPort );
    }
  };

  auto& bridge = j["Bridge"];
  for ( json::const_iterator iterBridge = bridge.begin(); bridge.end() != iterBridge; iterBridge++ ) {
    uuid_t uuidBridge( iterBridge.key() );
    bridge_t& br( m_mapBridge[ uuidBridge ] );  // assumes already exists
    auto& values = iterBridge.value()[ "new" ];
    //std::cout << values.dump(2) << std::endl;
    br.datapath_id = values[ "datapath_id" ];
    if ( values[ "fail_mode" ].is_string() ) {
      br.fail_mode = values[ "fail_mode" ];
    }
    br.name = values[ "name" ];
    br.stp_enable = values[ "stp_enable" ];

    auto& ports = values[ "ports" ];
    for ( json::const_iterator iterElement = ports.begin(); ports.end() != iterElement; iterElement++ ) {
      if ( "uuid" == *iterElement ) {
        fAddPort( uuidBridge, br, ports );
      }
      if ( "set" == (*iterElement) ) {
        iterElement++;
        assert( ports.end() != iterElement );
        auto& set = *iterElement;
        for ( json::const_iterator iterPair = set.begin(); set.end() != iterPair; iterPair++ ) {
          fAddPort( uuidBridge, br, *iterPair );
        };
      }
    }

    if ( nullptr != m_ovsdb.m_f.fBridgeUpdate) m_ovsdb.m_f.fBridgeUpdate( uuidBridge, br );

  }

  return true;
}

bool decode_impl::parse_port( const json& j ) {

  //std::cout << j.dump(2) << std::endl;

  auto& ports = j[ "Port" ];
  for ( json::const_iterator iterPortObject = ports.begin(); ports.end() != iterPortObject; iterPortObject++ ) {
    uuid_t uuidPort = iterPortObject.key();

    auto iterPort = m_mapPort.find( uuidPort );
    assert ( m_mapPort.end() != iterPort );
    auto& port( iterPort->second );

    auto& age = iterPortObject.value();

    for ( json::const_iterator iterAgeObject = age.begin(); age.end() != iterAgeObject; iterAgeObject++ ) {
      if ( "new" == iterAgeObject.key() ) {
        auto& values = iterAgeObject.value();
        port.name = values[ "name" ];
        if ( values[ "tag" ].is_number() ) {
          port.tag = values[ "tag" ];
        }

        auto& trunks = values[ "trunks" ];
        for ( json::const_iterator iterElement = trunks.begin(); trunks.end() != iterElement; iterElement++ ) {
          assert( "set" == *iterElement );
          iterElement++;
          assert( trunks.end() != iterElement );
          assert( (*iterElement).is_array() );
          auto& vlans = *iterElement;
          for ( json::const_iterator iterVlan = vlans.begin(); vlans.end() != iterVlan; iterVlan++ ) {
            port.setTrunk.insert( std::set<uint16_t>::value_type( *iterVlan ) );
          }
        }

        if ( values[ "vlan_mode" ].is_string() ) {
          port.VlanMode = values[ "vlan_mode" ];
        }

        if ( nullptr != m_ovsdb.m_f.fPortUpdate ) {
          m_ovsdb.m_f.fPortUpdate( uuidPort, port );
        }

        auto& interfaces = values[ "interfaces" ];
        for ( json::const_iterator iterInterface = interfaces.begin(); interfaces.end() != iterInterface; iterInterface++ ) {
          if ( (*iterInterface).is_array() ) {
            auto& elements = *iterInterface;
            assert( 0 );  // need to fix this with bonding and such (use function lamda as above)
            for ( json::const_iterator iterElement = elements.begin(); elements.end() != iterElement; iterElement++ ) {
            }
          }
          else {
            assert( "uuid" == *iterInterface );
            iterInterface++;
            assert( (*iterInterface).is_string() );
            uuid_t uuidInterface( *iterInterface );
            m_mapInterface.insert( mapInterface_t::value_type( uuidInterface, interface_t() ) );
            port.setInterface.insert( setInterface_t::value_type( uuidInterface ) );
            if ( nullptr != m_ovsdb.m_f.fInterfaceAdd ) m_ovsdb.m_f.fInterfaceAdd( uuidPort, uuidInterface );
          }
        }
      }
      if ( "old" == iterAgeObject.key() ) {
        // TODO: to be performed for port removal
      }
    }
  } // for iterPortObject

  return true;
}

bool decode_impl::parse_interface( const json& j ) {

  //std::cout << j.dump(2) << std::endl;

  auto& interfaces = j["Interface"];

  for ( json::const_iterator iterInterfaceJson = interfaces.begin(); interfaces.end() != iterInterfaceJson; iterInterfaceJson++ ) {
    uuid_t uuidInterface = iterInterfaceJson.key();
    mapInterface_t::iterator iterInterface = m_mapInterface.find( uuidInterface );
    assert( m_mapInterface.end() != iterInterface );

    auto& age = iterInterfaceJson.value();

    for ( json::const_iterator iterAgeObject = age.begin(); age.end() != iterAgeObject; iterAgeObject++ ) {
      if ( "new" == iterAgeObject.key() ) {
        // TODO: use boost::spirit to decode the json values into this structure
        auto& interfaceJson = iterAgeObject.value();
        auto& interfaceMap( iterInterface->second );

        size_t cntNeeded( 0 );
        // for a creation, three steps: (using lxc-start as example)
        //   name only
        //   all with admin up, link down
        //   all with admin up, link up
        if ( interfaceJson[ "ofport" ].is_number_integer() ) {
          interfaceMap.ofport = interfaceJson[ "ofport" ];
          cntNeeded++;
        }
        if ( interfaceJson[ "ifindex" ].is_number_integer() ) {
          interfaceMap.ifindex = interfaceJson[ "ifindex" ];
        }
        if ( interfaceJson[ "name" ].is_string() ) {
          interfaceMap.name = interfaceJson[ "name" ];
          cntNeeded++;
        }
        if ( interfaceJson[ "admin_state" ].is_string() ) {
          interfaceMap.admin_state = interfaceJson[ "admin_state" ];
        }
        if ( interfaceJson[ "link_state" ].is_string() ) {
          interfaceMap.link_state = interfaceJson[ "link_state" ];
        }
        if ( interfaceJson[ "mac_in_use" ].is_string() ) {
          interfaceMap.mac_in_use = interfaceJson[ "mac_in_use" ];
          cntNeeded++;
        }
        if ( interfaceJson[ "type" ].is_string() ) {
          interfaceMap.ovs_type = interfaceJson[ "type" ];
        }

        if ( 3 <= cntNeeded ) {
          if ( nullptr != m_ovsdb.m_f.fInterfaceUpdate ) {
            m_ovsdb.m_f.fInterfaceUpdate( uuidInterface, interfaceMap );
          }
        }
      }
      if ( "old" == iterAgeObject.key() ) {
        // TODO: detect and process removal of interface
      }
    }
  }

  return true;
}

// TODO: may consider sending collection of statistics in one large message
bool decode_impl::parse_statistics( const json& j ) {

  auto& interfaces = j["Interface"];

  //std::cout << interfaces.dump(2) << std::endl;

  for ( json::const_iterator iterInterfaceJson = interfaces.begin(); interfaces.end() != iterInterfaceJson; iterInterfaceJson++ ) {
    uuid_t uuidInterface = iterInterfaceJson.key();
    std::cout << "ovsdb_impl::parse_statistics: " << uuidInterface << std::endl;
    mapInterface_t::iterator iterInterface = m_mapInterface.find( uuidInterface );
    assert( m_mapInterface.end() != iterInterface );

    auto& age = iterInterfaceJson.value();

    for( json::const_iterator iterAgeObject = age.begin(); age.end() != iterAgeObject; iterAgeObject++ ) {
      if ( "new" == iterAgeObject.key() ) {
        // TODO: use boost::spirit to decode the json values into this structure
        auto& interfaceJson = iterAgeObject.value();
        auto& interfaceMap( iterInterface->second );

        // page 112 of openflow 1.4.1 spec shows how to get statistics via the controller channel
        //   therefore, this may go away at some point
        auto elements = interfaceJson[ "statistics" ];

        for ( json::iterator iterElements = elements.begin(); elements.end() != iterElements; iterElements++ ) {
          assert( "map" == *iterElements );
          iterElements++;
          assert( (*iterElements).is_array() );
          auto& statistics = *iterElements;
          //std::cout << "===" << statistics.dump() << std::endl;
          for ( json::iterator iterCombo = statistics.begin(); statistics.end() != iterCombo; iterCombo++ ) {
            mapStatistics_t& map( interfaceMap.mapStatistics );
            for ( json::iterator iterStatistic = (*iterCombo).begin(); (*iterCombo).end() != iterStatistic; iterStatistic++ ) {
              std::string name( *iterStatistic );
              iterStatistic++;
              mapStatistics_t::iterator iter = map.find( name );
              if ( map.end() == iter ) {
                bool bFound( false );
                // TODO: construct via macro to ensure consistency
                // take a look at the spirit example:
                //   https://www.boost.org/doc/libs/1_64_0/libs/spirit/doc/html/spirit/qi/tutorials/complex___our_first_complex_parser.html
                if ( "collisions"   == name ) iter = map.insert( map.begin(), mapStatistics_t::value_type( name, interfaceMap.statistics->collisions ) ); bFound = true;
                if ( "rx_bytes"     == name ) iter = map.insert( map.begin(), mapStatistics_t::value_type( name, interfaceMap.statistics->rx_bytes ) ); bFound = true;
                if ( "rx_crc_err"   == name ) iter = map.insert( map.begin(), mapStatistics_t::value_type( name, interfaceMap.statistics->rx_crc_err ) ); bFound = true;
                if ( "rx_dropped"   == name ) iter = map.insert( map.begin(), mapStatistics_t::value_type( name, interfaceMap.statistics->rx_dropped ) ); bFound = true;
                if ( "rx_errors"    == name ) iter = map.insert( map.begin(), mapStatistics_t::value_type( name, interfaceMap.statistics->rx_errors ) ); bFound = true;
                if ( "rx_frame_err" == name ) iter = map.insert( map.begin(), mapStatistics_t::value_type( name, interfaceMap.statistics->rx_frame_err ) ); bFound = true;
                if ( "rx_over_err"  == name ) iter = map.insert( map.begin(), mapStatistics_t::value_type( name, interfaceMap.statistics->rx_over_err ) ); bFound = true;
                if ( "rx_packets"   == name ) iter = map.insert( map.begin(), mapStatistics_t::value_type( name, interfaceMap.statistics->rx_packets ) ); bFound = true;
                if ( "tx_bytes"     == name ) iter = map.insert( map.begin(), mapStatistics_t::value_type( name, interfaceMap.statistics->tx_bytes ) ); bFound = true;
                if ( "tx_dropped"   == name ) iter = map.insert( map.begin(), mapStatistics_t::value_type( name, interfaceMap.statistics->tx_dropped ) ); bFound = true;
                if ( "tx_errors"    == name ) iter = map.insert( map.begin(), mapStatistics_t::value_type( name, interfaceMap.statistics->tx_errors ) ); bFound = true;
                if ( "tx_packets"   == name ) iter = map.insert( map.begin(), mapStatistics_t::value_type( name, interfaceMap.statistics->tx_packets ) ); bFound = true;
                if ( !bFound ) std::cout << "ovsdb_impl::parse_statistics did not find " << name << std::endl;
              }
              iter->second = *iterStatistic; // place into statistics_t via reference in map
            }
          }
        }

        if ( nullptr != m_ovsdb.m_f.fStatisticsUpdate ) {
          m_ovsdb.m_f.fStatisticsUpdate( uuidInterface, *interfaceMap.statistics );
        }
      }
      if ( "old" == iterAgeObject.key() ) {
        // TODO: last of the statistics are presented
      }
    }


  }

  return true;
}

void decode_impl::parse( vByte_t::const_iterator begin, size_t lenRead ) {

  json j;
  try {
    j = json::parse( begin, begin + lenRead );
    //std::cout << j.dump(2) << std::endl;

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
                    parse_bridge( items );  // format is different, more testing
            }
            if ( "port" == key ) {
                    parse_port( items );  // format is different, more testing
            }
            if ( "interface" == key ) {
                    parse_interface( items );  // format is different, more testing
            }
            if ( "statistics" == key ) {
                    parse_statistics( items );  // format is different, more testing
            }
          } );
        }
        break;
      case stuck:
        std::cout << "ovsdb arrived in stuck state" << std::endl;
        break;
    }
  }
  catch ( json::parse_error& e ) {
    if ( 101 == e.id ) {
      if ( nullptr != strstr( e.what(), "unexpected '{'" ) ) {
        // need to recursively split the string and try again
        //std::cout << "*** decode_impl::parse: splitting json " << lenRead << std::endl;
        //std::cout << "   " << e.what() << std::endl;
        parse( begin, e.byte - 1 );
        parse( begin + e.byte - 1, lenRead - e.byte + 1 );
      }
      else assert( 0 );
    }
    else assert( 0 );
  }

}

void decode_impl::do_read() {
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

          parse( m_vRx.begin(), lenRead );
          std::cout << ">>> ovsdb read end." << std::endl;
        }
        do_read();
      });
}

} // namespace ovsdb