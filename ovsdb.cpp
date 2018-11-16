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

#include <iostream>
#include <algorithm>

#include <boost/asio/write.hpp>

#include <json.hpp>

#include "ovsdb.h"

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

  // send first query  
  m_state = listdb;
  
  json j = {
    { "method", "list_dbs" },
    { "params", json::array() },
    { "id", 1 }
  };
  //std::cout << "*** test output: " << j << std::endl;
  send( j.dump() );
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
              m_state = listdb;
              break;
            case listdb: {
                m_state = stuck;
                assert( 1 == j["id"] );
                //std::cout << "listdb entries: ";
                auto result = j["result"];
                if ( result.is_array() ) {
                  for ( json::iterator iter = result.begin(); iter != result.end(); iter++ ) {
                    //std::cout << *iter;
                    if ( "Open_vSwitch" == *iter ) {
                      m_state = monitorBridge;
                    }
                  }
                }
                //std::cout << std::endl;
                
                // send next query
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
              break;
            case monitorBridge: {
                m_state = stuck;
                std::cout << "*** processing bridge" << std::endl;
                assert( 2 == j["id"] );
                auto result = j["result"];
                auto ovs = result["Open_vSwitch"];
                for ( json::iterator iterOvs = ovs.begin(); iterOvs != ovs.end(); iterOvs++ ) {
                  std::string uuid = iterOvs.key();
                  auto value = iterOvs.value();
                  auto new_ = value["new"];
                  //std::cout << new_.dump(2) << std::endl;
                  std::string db_version = new_["db_version"];
                  std::string ovs_version = new_["ovs_version"];
                  auto external_ids = new_["external_ids"];
                  if ( external_ids.is_array() ) {
                    for ( json::iterator iterId = external_ids.begin(); iterId != external_ids.end(); iterId++ ) {
                      if ( "map" == *iterId ) {
                        iterId++;
                        auto elements = *iterId;
                        size_t cnt = elements.size();
                        std::string hostname;
                        for ( json::iterator iterElement = elements.begin(); iterElement != elements.end(); iterElement++ ) {
                          if ( "hostname" == (*iterElement)[0] ) {
                            hostname = (*iterElement)[1];
                          } 
                        }
                      }
                    }
                  }
                  auto bridges = new_["bridges"];
                  size_t cntBridges = bridges.size();
                  std::vector<std::string> vBridgeUuid;
                  for ( json::iterator iterBridge = bridges.begin(); iterBridge != bridges.end(); iterBridge++ ) {
                    if ( (*iterBridge).is_array() ) {
                      assert( 0 );  // need to process multiple bridges
                    }
                    else {
                      assert( "uuid" == (*iterBridge) );
                      iterBridge++;
                      vBridgeUuid.push_back( (*iterBridge) );
                    }
                  }
                }
                
                // send next query
                json colInterface;
                json keys = json::object();
                
                colInterface["columns"] = { "admin_state", "link_state", "name", "ofport", "ifindex","mac_in_use", "type", "statistics" };
                keys["Interface"]       = json::array( { colInterface } );
                
                json j = {
                  { "id", 3 },
                  { "method", "monitor" },
                  { "params", { "Open_vSwitch", json::array( { "interface" } ), keys } }
                };
                send( j.dump() );
                
                m_state = monitorInterface;
              }
              break;
            case monitorInterface: {
                m_state = monitorPort;
              
                // send next query
                json idMonitor = json::array( { "port" } );
                
                json colPort;
                json keys = json::object();
                
                colPort["columns"] = { "interfaces", "name", "tag", "trunks", "vlan_mode" };
                keys["Port"]       = json::array( { colPort } );
                
                json j = {
                  { "id", 4 },
                  { "method", "monitor" },
                  { "params", { "Open_vSwitch", idMonitor, keys } }
                };
                send( j.dump() );
              }
              break;
            case monitorPort:
              m_state = listen;
              break;
            case listen:
              break;
            case stuck:
              std::cout << "arrived in stuck state" << std::endl;
              break;
          }
        }
        do_read();
      });
}