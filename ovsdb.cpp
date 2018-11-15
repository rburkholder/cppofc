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

#include "ovsdb.h"

namespace {
const char* szQuery[] = {
  "{\"method\":\"list_dbs\", \"params\":[], \"id\":1}",
// ==
  "{\"method\":\"monitor\", "
  "\"params\":[\"Open_vSwitch\",[\"bridge\"],"
    "{"
      "\"Open_vSwitch\":[{\"columns\":[\"bridges\",\"db_version\",\"ovs_version\",\"external_ids\"]}],"
      "\"Bridge\":[{\"columns\":[\"datapath_id\",\"fail_mode\",\"name\",\"ports\",\"stp_enable\"]}]"
    "}], "
  "\"id\":2}" ,
// ==
  "{\"method\":\"monitor\", "
  "\"params\":[\"Open_vSwitch\",[\"interface\"],"
    "{"
      "\"Interface\":[{\"columns\":[\"admin_state\",\"link_state\",\"name\",\"ofport\",\"ifindex\",\"mac_in_use\",\"type\",\"statistics\"]}]"
    "}], "
  "\"id\":3}", 
// ==
  "{\"method\":\"monitor\", "
  "\"params\":[\"Open_vSwitch\",[\"port\"],"
    "{"
      "\"Port\":[{\"columns\":[\"interfaces\",\"name\",\"tag\",\"trunks\",\"vlan_mode\"]}]"
    "}], "
  "\"id\":4}"
};
}

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
  send( szQuery[ 0 ] );
}

ovsdb::~ovsdb( ) {
}

void ovsdb::send( const char* sz ) {
  try {
    std::string sCmd( sz );
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
          std::for_each( m_vRx.begin(), m_vRx.end(), [](char ch){
            std::cout << ch;
          });
          std::cout << std::endl;
          std::cout << ">>> ovsdb read end." << std::endl;
        }
        switch ( m_state ) {
          case start:
            m_state = listdb;
            //send( szQuery[ 0 ] );
            break;
          case listdb:
            m_state = monitorBridge;
            send( szQuery[ 1 ] );
            break;
          case monitorBridge:
            m_state = monitorInterface;
            send( szQuery[ 2 ] );
            break;
          case monitorInterface:
            m_state = monitorPort;
            send( szQuery[ 3 ] );
            break;
          case monitorPort:
            m_state = listen;
            break;
          case listen:
            break;
        }
        do_read();
      });
}