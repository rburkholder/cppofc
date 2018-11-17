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

namespace asio = boost::asio;
using json = nlohmann::json;

class ovsdb_impl {
public:
  ovsdb_impl( asio::io_context& io_context );
  virtual ~ovsdb_impl( );
protected:
private:
  
  enum { max_length = 65540 };  // total header and data for ipv4 is 65535
  
  asio::local::stream_protocol::endpoint m_ep;
  asio::local::stream_protocol::socket m_socket;
  
  vByte_t m_vRx;
  
  enum EState { start, listdb, monitorBridge, monitorInterface, monitorPort, listen, stuck };
  
  EState m_state;
  
  // ----
  // TODO: use boost::spirit to decode the json values into this structure
  struct statistics_t {
    size_t collisions;
    size_t rx_bytes;
    size_t rx_crc_err;
    size_t rx_dropped;
    size_t rx_errors;
    size_t rx_frame_err;
    size_t rx_over_err;
    size_t rx_packets;
    size_t tx_bytes;
    size_t tx_dropped;
    size_t tx_errors;
    size_t tx_packets;
    statistics_t(): 
      collisions {}, 
      rx_bytes {}, rx_crc_err {}, rx_dropped {}, rx_errors {}, rx_frame_err {}, rx_over_err {}, rx_packets {},
      tx_bytes {}, tx_dropped {}, tx_errors {}, tx_packets {}
      {}
  };
  
  typedef std::map<std::string,size_t> mapStatistics_t;
  
  // ----
  struct interface_t {
    std::string name;
    std::string admin_state;
    std::string link_state;
    std::string mac_in_use;
    std::string ovs_type;
    size_t ifindex;
    size_t ofport;
    //statistics_t statistics;
    mapStatistics_t mapStatistics;
    interface_t(): ifindex {}, ofport {} {}
  };
  typedef std::set<std::string> setInterface_t; // uses uuid as key
  typedef std::map<std::string, interface_t> mapInterface_t;
  
  mapInterface_t m_mapInterface;
  
  // ----
  struct port_t {
    std::string name;
    setInterface_t setInterfaces;
    uint16_t tag; // access port
    std::set<uint16_t> setTrunks;
    std::set<uint16_t> setVlanMode;  // not sure content of this yet
  };
  typedef std::set<std::string> setPort_t;  // uses uuid as key
  typedef std::map<std::string,port_t> mapPort_t;
  
  mapPort_t m_mapPort;
  
  // ----
  struct bridge_t {
    //std::string uuid;
    std::string datapath_id;
    std::string fail_mode;
    std::string name;
    bool stp_enable;
    std::set<std::string> setPorts;
  };
  typedef std::map<std::string,bridge_t> mapBridge_t;
  
  // ----
  struct switch_t {
    std::string uuid;
    std::string db_version;
    std::string ovs_version;
    std::string hostname;
    mapBridge_t mapBridge;
  };
  
  switch_t m_switch;
  
  void send( const std::string& );
  void send_list_dbs();
  void send_monitor_bridges();
  void send_monitor_ports();
  void send_monitor_interfaces();
  void do_read();

  void parse_listdb( json& );
  void parse_bridge( json& );
  void parse_port( json& );
  void parse_interface( json& );
};

#endif /* OVSDB_H */

