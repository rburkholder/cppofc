/* 
 * File:   ovsdb_impl.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net *
 *
 * Created on November 14, 2018, 5:04 PM
 */

// https://www.boost.org/doc/libs/1_68_0/doc/html/boost_asio/overview/posix/local.html

#ifndef OVSDB_IMPL_H
#define OVSDB_IMPL_H

#include <set>

// lsof|grep ovsdb

#include <boost/asio/local/stream_protocol.hpp>

#include <json.hpp>

#include "common.h"
#include "ovsdb.h"

namespace asio = boost::asio;
using json = nlohmann::json;

class ovsdb_impl {
public:
  ovsdb_impl( ovsdb&, asio::io_context& io_context );
  virtual ~ovsdb_impl( );
protected:
private:
  
  enum { max_length = 65540 };  // total header and data for ipv4 is 65535
  
  asio::local::stream_protocol::endpoint m_ep;
  asio::local::stream_protocol::socket m_socket;
  
  vByte_t m_vRx;
  
  enum EState { start, listdb, startBridgeMonitor, startInterfaceMonitor, startPortMonitor, listen, stuck };
  
  EState m_state;
  
  ovsdb& m_ovsdb;
  
  ovsdb::switch_t m_switch;
  ovsdb::mapPort_t m_mapPort;
  ovsdb::mapInterface_t m_mapInterface;

  void send( const std::string& );
  void send_list_dbs();
  void send_monitor_bridges();
  void send_monitor_ports();
  void send_monitor_interfaces();
  void do_read();

  bool parse_listdb( json& );
  bool parse_bridge( json& );
  bool parse_port( json& );
  bool parse_interface( json& );
};

#endif /* OVSDB_H */

