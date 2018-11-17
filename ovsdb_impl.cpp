/* 
 * File:   ovsdb.cpp
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

#include <iostream>
#include <algorithm>

#include <boost/asio/write.hpp>

#include <json.hpp>

#include "ovsdb_impl.h"

using json = nlohmann::json;

ovsdb::ovsdb( asio::io_context& io_context )
:
  m_ep( "/var/run/openvswitch/db.sock" ),
  //m_ep( ip::tcp::v4(), 6640 ),
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

ovsdb::~ovsdb( ) {
}

// TODO: once all query strings converted over, then change this to a const std::string
void ovsdb::send( const std::string& sCmd ) {
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

void ovsdb::send_list_dbs() {
  json j = {
    { "method", "list_dbs" },
    { "params", json::array() },
    { "id", 1 }
  };
  //std::cout << "*** test output: " << j << std::endl;
  send( j.dump() );
}

void ovsdb::send_monitor_bridges() {
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

void ovsdb::send_monitor_ports() {
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

void ovsdb::send_monitor_interfaces() {
  json colInterface;
  json keys = json::object();

  colInterface["columns"] = { "admin_state", "link_state", "name", "ofport", "ifindex","mac_in_use", "type", "statistics" };
  keys["Interface"]       = json::array( { colInterface } );

  json j = {
    { "id", 4 },
    { "method", "monitor" },
    { "params", { "Open_vSwitch", json::array( { "interface" } ), keys } }
  };

  send( j.dump() );
}

void ovsdb::do_read() {
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
          std::cout << j.dump(2) << std::endl;
          std::cout << ">>> ovsdb read end." << std::endl;
          
          // process read state
          switch ( m_state ) {
            case start:
              //m_state = listdb;
              std::cout << "*** something arrived in state: start" << std::endl;
              break;
            case listdb: {
                m_state = stuck;
                
                assert( j["error"].is_null() );
                assert( 1 == j["id"] );
                //std::cout << "listdb entries: ";
                auto& result = j["result"];
                if ( result.is_array() ) {
                  for ( json::iterator iter = result.begin(); iter != result.end(); iter++ ) {
                    //std::cout << *iter;
                    if ( "Open_vSwitch" == *iter ) {
                      
                      m_state = monitorBridge;
                      send_monitor_bridges();
                      
                    }
                  }
                }
                //std::cout << std::endl;
                
              }
              break;
            case monitorBridge: {
                m_state = stuck;
                
                assert( j["error"].is_null() );
                assert( 2 == j["id"] );
                auto& result = j["result"];

                // --
                auto& ovs = result["Open_vSwitch"];
                for ( json::iterator iterOvs = ovs.begin(); iterOvs != ovs.end(); iterOvs++ ) {
                  m_switch.uuid = iterOvs.key();
                  auto& values = iterOvs.value()["new"];
                  //std::cout << values.dump(2) << std::endl;
                  m_switch.db_version = values["db_version"];
                  m_switch.ovs_version = values["ovs_version"];
                  auto& external_ids = values["external_ids"];
                  if ( external_ids.is_array() ) {
                    for ( json::iterator iterId = external_ids.begin(); iterId != external_ids.end(); iterId++ ) {
                      if ( "map" == *iterId ) {
                        iterId++;
                        auto& elements = *iterId;
                        size_t cnt = elements.size();
                        std::string hostname;
                        for ( json::iterator iterElement = elements.begin(); iterElement != elements.end(); iterElement++ ) {
                          if ( "hostname" == (*iterElement)[0] ) {
                            m_switch.hostname = (*iterElement)[1];
                          } 
                        }
                      }
                    }
                  }
                  
                  auto fAddBridge = [this](json& j){
                    for ( json::iterator iterBridge = j.begin(); j.end() != iterBridge; iterBridge++ ) {
                      assert( "uuid" == (*iterBridge) );
                      iterBridge++;
                      m_switch.mapBridge.insert( mapBridge_t::value_type( *iterBridge, bridge_t() ) );
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
                }

                // --
                auto& bridge = result["Bridge"];
                for ( json::iterator iterBridge = bridge.begin(); bridge.end() != iterBridge; iterBridge++ ) {
                  bridge_t& br( m_switch.mapBridge[ iterBridge.key() ] );
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
                          m_mapPort.insert( mapPort_t::value_type( *iterPort, port_t() ) );
                          br.setPorts.insert( setPort_t::value_type( *iterPort ) );
                        }
                      };
                    }
                  }
                }
                
                m_state = monitorPort;
                send_monitor_ports();
                
              }
              break;
            case monitorPort: {
                m_state = stuck;
                
                assert( j["error"].is_null() );
                assert( 3 == j["id"] );
                auto& result = j["result"];
                
                auto& ports = result[ "Port" ];
                for ( json::iterator iterPortObject = ports.begin(); ports.end() != iterPortObject; iterPortObject++ ) {
                  auto iterPort = m_mapPort.find( iterPortObject.key() );
                  assert ( m_mapPort.end() != iterPort );
                  auto& values = iterPortObject.value()[ "new" ];
                  iterPort->second.name = values[ "name" ];
                  if ( values[ "tag" ].is_number() ) {
                    iterPort->second.tag = values[ "tag" ];
                  }
                  
                  auto& trunks = values[ "trunks" ];
                  for ( json::iterator iterElement = trunks.begin(); trunks.end() != iterElement; iterElement++ ) {
                    assert( "set" == *iterElement );
                    iterElement++;
                    assert( trunks.end() != iterElement );
                    assert( (*iterElement).is_array() );
                    auto& vlans = *iterElement;
                    for ( json::iterator iterVlan = vlans.begin(); vlans.end() != iterVlan; iterVlan++ ) {
                      iterPort->second.setTrunks.insert( std::set<uint16_t>::value_type( *iterVlan ) );
                    }
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
                      m_mapInterface.insert( mapInterface_t::value_type( *iterInterface, interface_t() ) );
                      iterPort->second.setInterfaces.insert( setInterface_t::value_type( *iterInterface ) );
                    }
                  }
                }
              
                m_state = monitorInterface;
                send_monitor_interfaces();
                
              }
              break;
            case monitorInterface: {
              
                assert( j["error"].is_null() );
                assert( 4 == j["id"] ); // will an update inter-leave here?  
                  // should we just do a big switch on in coming id's to be more flexible?
                  // then mark a vector of flags to indicate that it has been processed?
                
                auto& interfaces = j["result"]["Interface"];

                for ( json::iterator iterInterfaceJson = interfaces.begin(); interfaces.end() != iterInterfaceJson; iterInterfaceJson++ ) {
                  std::string uuid = iterInterfaceJson.key();
                  mapInterface_t::iterator iterInterface = m_mapInterface.find( uuid );
                  assert( m_mapInterface.end() != iterInterface );

                  auto& interfaceJson = iterInterfaceJson.value()[ "new" ];
                  auto& interfaceMap( iterInterface->second );
                  interfaceMap.ifindex     =            interfaceJson[ "ifindex" ];
                  interfaceMap.ofport      =            interfaceJson[ "ofport" ];
                  interfaceMap.name        = std::move( interfaceJson[ "name" ] );
                  interfaceMap.admin_state = std::move( interfaceJson[ "admin_state" ] );
                  interfaceMap.link_state  = std::move( interfaceJson[ "link_state" ] );
                  interfaceMap.mac_in_use  = std::move( interfaceJson[ "mac_in_use" ] );
                  interfaceMap.ovs_type    = std::move( interfaceJson[ "type" ] );
                  
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
                }

                m_state = listen;
              }
              break;
            case listen:
              // process the monitor/update message
              break;
            case stuck:
              std::cout << "arrived in stuck state" << std::endl;
              break;
          }
        }
        do_read();
      });
}