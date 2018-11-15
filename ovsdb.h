/* 
 * File:   ovsdb.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net *
 *
 * Created on November 14, 2018, 5:04 PM
 */

// https://www.boost.org/doc/libs/1_68_0/doc/html/boost_asio/overview/posix/local.html

#ifndef OVSDB_H
#define OVSDB_H

// lsof|grep ovsdb

#include <boost/asio/local/stream_protocol.hpp>

//#include <boost/asio/ip/tcp.hpp>

#include "common.h"

namespace asio = boost::asio;
//namespace ip = boost::asio::ip;

class ovsdb {
public:
  ovsdb( asio::io_context& io_context );
  virtual ~ovsdb( );
protected:
private:
  
  enum { max_length = 65540 };  // total header and data for ipv4 is 65535
  
  //ip::tcp::endpoint m_ep;
  //ip::tcp::socket m_socket;
  
  asio::local::stream_protocol::endpoint m_ep;
  asio::local::stream_protocol::socket m_socket;
  
  vByte_t m_vRx;
  
  void do_read();
};

#endif /* OVSDB_H */

