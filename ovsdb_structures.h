/*
 * File:   ovsdb_structures.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net *
 *
 * Created on November 26, 2018, 1:58 PM
 */

#ifndef OVSDB_STRUCTURES_H
#define OVSDB_STRUCTURES_H

#include <map>
#include <set>
#include <functional>

namespace ovsdb {
namespace structures {

  typedef std::string uuid_t;

  typedef uuid_t uuidSwitch_t;
  typedef uuid_t uuidBridge_t;
  typedef uuid_t uuidPort_t;
  typedef uuid_t uuidInterface_t;

  // ----
  struct switch_t {
    std::string hostname;
    std::string ovs_version;
    std::string db_version;
  };

  struct bridge_t {
    std::string name;
    std::string datapath_id;
    std::string fail_mode;
    bool stp_enable;
    bridge_t(): stp_enable( false ) {}
  };

  struct port_t { // ports from a range of bridges
    std::string name;
    uint16_t tag; // port access vlan
    std::set<uint16_t> setTrunk; // a set of vlan numbers
    std::set<uint16_t> setVlanMode;  // not sure content of this yet
    port_t(): tag {} {}
  };

  struct interface_t { // interfaces from a range of ports
    std::string name;
    std::string ovs_type;
    std::string admin_state;
    std::string link_state;
    std::string mac_in_use;
    size_t ifindex;
    size_t ofport;
    interface_t(): ifindex {}, ofport {} {}
  };

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

  typedef std::function<void(const uuidSwitch_t&)> fSwitchAdd_t;
  typedef std::function<void(const uuidSwitch_t&,const switch_t&)> fSwitchUpdate_t;
  typedef std::function<void(const uuidSwitch_t&)> fSwitchDelete_t;  // to be implemented

  typedef std::function<void(const uuidSwitch_t&, const uuidBridge_t&)> fBridgeAdd_t;
  typedef std::function<void(const uuidBridge_t&,const bridge_t&)> fBridgeUpdate_t;
  typedef std::function<void(const uuidBridge_t&)> fBridgeDelete_t;  // to be implemented

  typedef std::function<void(const uuidBridge_t&, const uuidPort_t&)> fPortAdd_t;
  typedef std::function<void(const uuidPort_t&,const port_t&)> fPortUpdate_t;
  typedef std::function<void(const uuidPort_t&)> fPortDelete_t;  // to be implemented

  typedef std::function<void(const uuidPort_t&, const uuidInterface_t&)> fInterfaceAdd_t;
  typedef std::function<void(const uuidInterface_t&,const interface_t&)> fInterfaceUpdate_t;
  typedef std::function<void(const uuidInterface_t&)> fInterfaceDelete_t;  // to be implemented

  typedef std::function<void(const uuidInterface_t&,const statistics_t&)> fStatisticsUpdate_t;

  struct f_t {
    fSwitchAdd_t        fSwitchAdd;
    fSwitchUpdate_t     fSwitchUpdate;
    fSwitchDelete_t     fSwitchDelete;

    fBridgeAdd_t        fBridgeAdd;
    fBridgeUpdate_t     fBridgeUpdate;
    fBridgeDelete_t     fBridgeDelete;

    fPortAdd_t          fPortAdd;
    fPortUpdate_t       fPortUpdate;
    fPortDelete_t       fPortDelete;

    fInterfaceAdd_t     fInterfaceAdd;
    fInterfaceUpdate_t  fInterfaceUpdate;
    fInterfaceDelete_t  fInterfaceDelete;

    fStatisticsUpdate_t fStatisticsUpdate;
  };

} // namespace structures
} // namespace ovsdb

#endif /* OVSDB_STRUCTURES_H */

