/* 
 * File:   ovsdb.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net *
 * 
 * Created on November 14, 2018, 5:04 PM
 */

// https://tools.ietf.org/html/rfc7047
// http://www.openvswitch.org//ovs-vswitchd.conf.db.5.pdf

#include <iostream>
#include <algorithm>

#include <boost/asio/write.hpp>

#include "ovsdb.h"

ovsdb::ovsdb( asio::io_context& io_context )
:
  m_ep( "/var/run/openvswitch/db.sock" ),
  //m_ep( ip::tcp::v4(), 6640 ),
  m_socket( io_context )
{
  assert( BOOST_ASIO_HAS_LOCAL_SOCKETS );
  m_socket.connect( m_ep );
  
  try {
    std::string sCmd( "{\"method\":\"list_dbs\", \"params\":[], \"id\":1}" );
    asio::async_write( 
      m_socket, boost::asio::buffer( sCmd ), 
      [this](boost::system::error_code ec, std::size_t cntWritten ){
        if ( ec ) {
          std::cout << "<<< ovsdb write error: " << ec.message() << std::endl;
        }
        else {
          std::cout << "<<< ovsdb written " << cntWritten << std::endl;
        }
      } );
    
  }
  catch ( std::exception& e ) {
    std::cout << "<<< ovsdb error: " << e.what() << std::endl;
  }
  
  do_read(); // start up socket read 
  //(will need to start some commands, but examine what happens so far)
}

ovsdb::~ovsdb( ) {
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
        do_read();
      });
}