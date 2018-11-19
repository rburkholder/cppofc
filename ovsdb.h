/* 
 * File:   ovsdb.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net *
 *
 * Created on November 17, 2018, 11:25 AM
 */

#ifndef OVSDB_H
#define OVSDB_H

#include <memory>
#include <functional>

#include <boost/asio/io_context.hpp>

namespace asio = boost::asio;

class ovsdb_impl;

class ovsdb {
  friend class ovsdb_impl;
public:

  // structure not used yet, used once strings are processed by boost::spririt
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
      rx_bytes {}, rx_packets {}, rx_dropped {}, rx_errors {}, rx_crc_err {}, rx_frame_err {}, rx_over_err {}, 
      tx_bytes {}, tx_packets {}, tx_dropped {}, tx_errors {}
      {}
  };
  
  typedef std::map<std::string,size_t> mapStatistics_t;
  
  struct interface_t { // interfaces from a range of ports
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

  struct port_t { // ports from a range of bridges
    std::string name;
    setInterface_t setInterfaces;
    uint16_t tag; // port access vlan
    std::set<uint16_t> setTrunks; // a set of vlan numbers
    std::set<uint16_t> setVlanMode;  // not sure content of this yet
  };

  typedef std::set<std::string> setPort_t;  // uses uuid as key
  typedef std::map<std::string,port_t> mapPort_t;

  struct bridge_t {
    std::string datapath_id;
    std::string fail_mode;
    std::string name;
    bool stp_enable;
    std::set<std::string> setPorts; // by uuid
  };

  typedef std::map<std::string,bridge_t> mapBridge_t;  // uses uuid as key

  // ----
  struct switch_t {
    std::string uuid;
    std::string db_version;
    std::string ovs_version;
    std::string hostname;
    mapBridge_t mapBridge; // by uuid
  };
  
  typedef std::function<void(const switch_t&)> fSwitchUpdate_t; // strings are movable
  typedef std::function<void(const mapPort_t&)> fPortUpdate_t;  // strings are movable
  typedef std::function<void(const mapInterface_t&)> fInterfaceUpdate_t; // strings are movable
  typedef std::function<void(const mapInterface_t&)> fStatisticsUpdate_t; // strings are movable
  
  ovsdb( 
    asio::io_context&,
    fSwitchUpdate_t,
    fPortUpdate_t,
    fInterfaceUpdate_t,
    fStatisticsUpdate_t
    );
  virtual ~ovsdb( );
  
protected:
private:
  
  typedef std::shared_ptr<ovsdb_impl> povsdb_impl_t;
  povsdb_impl_t m_ovsdb_impl;

  // TOOD: may keep a message queue of all messages (other than statistics)
  //   allows late-comers to obtain state transitions to current state
  //   provides message based mechanism for syncing, rather than locking the structure
  //  or run a strand for presenting updates, and requesting state and updates

  // TODO: these functions need to be assigned on construction
  //   allows them to be called with initial settings

  fSwitchUpdate_t m_fSwitchUpdate;
  fPortUpdate_t m_fPortUpdate;
  fInterfaceUpdate_t m_fInterfaceUpdate;
  fStatisticsUpdate_t m_fStatisticsUpdate;

  // TODO convert above functions to something more specific:
  /*
   * switch: add, update
   * bridge: add, update, delete
   * port: add update, delete
   * interface: add, update, delete
   * statistics: update
   */

};

#endif /* OVSDB_H */

