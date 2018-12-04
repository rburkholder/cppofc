/*
 * File:   bridge.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on September 22, 2017, 6:05 PM
 */

#ifndef BRIDGE_H
#define BRIDGE_H

#include <set>
#include <map>
#include <mutex>
#include <string>
#include <functional>
#include <unordered_map>

#include "common.h"
#include "mac.h"

// TODO: add io_context
//   allows a strand to be added and used for syncing updates from ovsdb and requests from tcp_session
//   which means that a pipeline of packet processing is required from tcp_session.
//   will need to do this any way to deal with packets requiring long duration processing such as
//      dns lookups, squid lookups, ....

// TODO: in tcp_session decodes, confirm access port is not receiving trunk info

class Bridge {
public:

  typedef size_t ofport_t;
  typedef uint16_t vlanid_t;

  typedef std::function<vByte_t(void)> fAcquireBuffer_t;
  typedef std::function<void(vByte_t)> fTransmitBuffer_t;

  enum OpState { unknOpState, up, down };
  enum MacStatus { StatusQuo, Multicast, Broadcast, Learned, Moved }; // add 'Flap' ?
  enum VlanMode { access, trunk, dot1q_tunnel, native_tagged, native_untagged };

  struct interface_t {
    uint16_t tag; // port access vlan
    std::set<vlanid_t> setTrunk; // a set of vlan numbers
    VlanMode eVlanMode;  // not sure content of this yet
    OpState admin_state;
    OpState link_state;
    mac_t mac_in_use;
    size_t ifindex;   // id from 'ip link'
    ofport_t ofport;  // openflow port id
    interface_t()
      : tag( 0 ), admin_state( OpState::unknOpState ), link_state( OpState::unknOpState ), ifindex( 0 ), ofport( 0 )
    {}
  };

  Bridge( );
  virtual ~Bridge( );

  // from ovsdb:
//  void AddInterface( const interface_t& ); // used embedded ofport as index
  void UpdateInterface( const interface_t& );
  void DelInterface( ofport_t );
  void UpdateState( ofport_t, OpState admin_state, OpState link_state );

  // from tcp_session on putting more smarts into bridge:
  void StartRulesInjection( fAcquireBuffer_t, fTransmitBuffer_t );

  // currently from tcp_session wondering how to forward packets
  MacStatus Update( nPort_t nPort, const mac_t& macSource );
  nPort_t Lookup( const mac_t& mac );

private:

  //enum typeVlan { unknVlan=0, access=1, trunk=2 };

  struct MacInfo {
    nPort_t m_inPort;
    MacInfo( nPort_t inPort ): m_inPort( inPort ) {}
  };

  typedef std::unordered_map<MacAddress,MacInfo> mapMac_t;
  mapMac_t m_mapMac;

  typedef std::map<ofport_t,interface_t> mapInterface_t;
  mapInterface_t m_mapInterface;

  typedef std::set<ofport_t> setPort_t;

  struct vlan_t {
    setPort_t setPortAccess; // set of ofport_t as access
    setPort_t setPortTrunk;  // set of ofport_t as trunk
    uint32_t idGroupAccess; // openflow group for access ports
    uint32_t idGroupTrunk;  // openflow group for trunk ports
    vlan_t(): idGroupAccess( 0 ), idGroupTrunk( 0 ) {}
  };

  typedef std::map<vlanid_t,vlan_t> mapVlanToPort_t;

  std::mutex m_mutex;

  bool m_bRulesInjectionActive;

  fAcquireBuffer_t m_fAcquireBuffer;
  fTransmitBuffer_t m_fTransmitBuffer;

  mapVlanToPort_t m_mapVlanToPort;
  setPort_t m_setPortWithAllVlans;

};

#endif /* BRIDGE_H */

