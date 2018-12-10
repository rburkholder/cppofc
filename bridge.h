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
#include "protocol/ethernet/mac.h"

// TODO: add io_context
//   allows a strand to be added and used for syncing updates from ovsdb and requests from tcp_session
//   which means that a pipeline of packet processing is required from tcp_session.
//   will need to do this any way to deal with packets requiring long duration processing such as
//      dns lookups, squid lookups, ....

// TODO: in tcp_session decodes, confirm access port is not receiving trunk info

class Bridge {
public:

  typedef size_t ofport_t;
  typedef uint16_t idVlan_t;
  typedef uint32_t idGroup_t;

  typedef std::set<idVlan_t> setVlan_t;
  typedef std::set<ofport_t> setPort_t;

  typedef std::function<vByte_t(void)> fAcquireBuffer_t;
  typedef std::function<void(vByte_t)> fTransmitBuffer_t;

  enum OpState { unknOpState, up, down };
  enum MacStatus { StatusQuo, Multicast, Broadcast, Learned, Moved }; // add 'Flap' ?
  enum VlanMode { access, trunk, dot1q_tunnel, native_tagged, native_untagged };

  struct interface_t {
    idVlan_t tag; // port access vlan; TODO: test tag is not member of trunk
    setVlan_t setTrunk; // a set of vlan numbers
    VlanMode eVlanMode;
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
  MacStatus Update( nPort_t nPort, idVlan_t idVlan, const MacAddress& macSource );
  void Forward( ofport_t ofp_ingress, idVlan_t vlan,
                const MacAddress& macSrc, const MacAddress& macDst,
                uint8_t* pPacket, size_t nOctets
                );

private:

  struct MacInfo {
    nPort_t m_inPort;
    size_t m_cntMoved;
    setVlan_t m_setVlanEncountered;
    MacInfo( nPort_t inPort ): m_cntMoved( 0 ), m_inPort( inPort ) {}
  };

  typedef std::unordered_map<MacAddress,MacInfo> mapMac_t;
  mapMac_t m_mapMac;

  typedef std::map<ofport_t,interface_t> mapInterface_t;
  mapInterface_t m_mapInterface;

  // used for broadcast when destination port is unknown
  struct VlanToPort_t {
    setPort_t setPortAccess; // set of ofport_t as access
    setPort_t setPortTrunk;  // set of ofport_t as trunk
    bool bGroupAdded;
    bool bGroupNeedsUpdate;
    VlanToPort_t():
      bGroupAdded( false ), bGroupNeedsUpdate( false )
    {}
  };

  typedef std::map<idVlan_t,VlanToPort_t> mapVlanToPort_t;

  mapVlanToPort_t m_mapVlanToPort;

  bool m_bGroupTrunkAllAdded;
  setPort_t m_setPortWithAllVlans;

  std::mutex m_mutex;

  bool m_bRulesInjectionActive;

  fAcquireBuffer_t m_fAcquireBuffer;
  fTransmitBuffer_t m_fTransmitBuffer;

  void BuildGroups();

};

#endif /* BRIDGE_H */

