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

  enum EState { start, listdb, startBridgeMonitor, startPortMonitor, startInterfaceMonitor, startStatisticsMonitor, listen, stuck };

  EState m_state;
  
  typedef ovsdb::uuid_t uuid_t;
  
  ovsdb& m_ovsdb;
  
  typedef std::map<std::string,size_t> mapStatistics_t;
  struct interface_t: public ovsdb::interface_t {
    mapStatistics_t mapStatistics;  // for now
    ovsdb::statistics_t statistics;  // for the future
  };
  typedef std::set<uuid_t> setInterface_t;
  
  struct port_t: public ovsdb::port_t {
    setInterface_t setInterface;
  };
  typedef std::set<uuid_t> setPort_t;
  
  struct bridge_t: public ovsdb::bridge_t {
    setPort_t setPort;
  };
  typedef std::set<uuid_t> setBridge_t;
  
  struct switch_t: public ovsdb::switch_t {
    setBridge_t setBridge;
  };

  typedef std::map<uuid_t,switch_t> mapSwitch_t;
  typedef std::map<uuid_t,bridge_t> mapBridge_t;
  typedef std::map<uuid_t,port_t> mapPort_t;
  typedef std::map<uuid_t,interface_t> mapInterface_t;
  
  
  mapSwitch_t m_mapSwitch;
  mapBridge_t m_mapBridge;
  mapPort_t m_mapPort;
  mapInterface_t m_mapInterface;

  void send( const std::string& );

  void send_list_dbs();
  void send_monitor_bridges();
  void send_monitor_ports();
  void send_monitor_interfaces();
  void send_monitor_statistics();

  void do_read();

  bool parse_listdb( json& );
  bool parse_bridge( json& );
  bool parse_port( json& );
  bool parse_interface( json& );
  bool parse_statistics( json& );
};

#endif /* OVSDB_H */

