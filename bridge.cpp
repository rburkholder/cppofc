/*
 * File:   bridge.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on September 22, 2017, 6:05 PM
 */

#include "hexdump.h"

#include "bridge.h"
#include "openflow/openflow-spec1.4.1.h"

#include "codecs/ofp_group_mod.h"
#include "codecs/ofp_flow_mod.h"
#include "codecs/ofp_packet_out.h"

namespace {
  // TODO: move this out to common?
  template<typename T>
  T* Append( vByte_t& v ) {
    size_t increment( sizeof( T ) );
    size_t placeholder( v.size() );
    size_t new_size = placeholder + increment;
    v.resize( new_size );
    T* p = new( v.data() + placeholder ) T;
    return p;
  }
}

Bridge::Bridge( )
: m_bRulesInjectionActive( false ), m_bGroupTrunkAllAdded( false )
{
  std::cout << "Bridge::Bridge construction" << std::endl;
}

Bridge::~Bridge( ) {
}

Bridge::MacStatus Bridge::Update( nPort_t nPort, idVlan_t idVlan, const MacAddress& macSource ) {

  if ( 0 == nPort ) {
    throw std::runtime_error( "Bridge::Update need port > 0" );
  }

  MacStatus status( StatusQuo );

  if ( macSource.IsMulticast() ) {
    status = Multicast;
    std::cout
      << "bridge: found source multicast " << macSource.Value()
      << " on port " << nPort
      << std::endl;
    // this is an illegal option, source mac cannot be multicast
  }
  else {
    if ( macSource.IsBroadcast() ) {
      status = Broadcast;
      std::cout
        << "bridge: found source broadcast " << macSource.Value()
        << " on port " << nPort
        << std::endl;
      // will need to evaluate this, and look at the meanings
    }
    else {
      mapMac_t::iterator iterMapMac = m_mapMac.find( macSource );
      if ( m_mapMac.end() == iterMapMac ) { // didn't find mac
        std::pair<MacAddress, MacInfo> pair( macSource, MacInfo( nPort ) );
        iterMapMac = m_mapMac.insert( m_mapMac.begin(), pair );
        status = Learned;
        std::cout
          << "bridge: mac " << HexDump<const uint8_t*>( macSource.Value(), macSource.Value() + 6, ':' )
          << " learned on port " << nPort
          << std::endl;
      }
      else {
        if ( nPort != iterMapMac->second.m_inPort ) { // mac moved (check for flap sometime)
          iterMapMac->second.m_inPort = nPort;
          iterMapMac->second.m_cntMoved++;
          status = Moved;
          std::cout
            << "bridge: mac " << HexDump<const uint8_t*>( macSource.Value(), macSource.Value() + 6, ':' )
            << " moved to port " << nPort
            << " flap count " << iterMapMac->second.m_cntMoved
            << std::endl;
        }
      }
      if ( 0 != idVlan ) {
        setVlan_t& setVlanEncountered( iterMapMac->second.m_setVlanEncountered );
        if ( 0 != setVlanEncountered.size() ) {
          setVlan_t::iterator iterSetVlan = setVlanEncountered.find( idVlan );
          if ( setVlanEncountered.end() == iterSetVlan ) {
            setVlanEncountered.insert( idVlan );
            std::cout
              << "bridge: mac " << HexDump<const uint8_t*>( macSource.Value(), macSource.Value() + 6, ':' )
              << " has vlan " << idVlan
              << std::endl;
          }
        }
      }
    }
  }

  return status;
}

nPort_t Bridge::Lookup( const MacAddress& mac ) {

  nPort_t nPort( ofp141::ofp_port_no::OFPP_ANY );  // neither ingress nor egress (pg 15)
  //MacAddress mac( mac_ );

  if ( mac.IsBroadcast()
    || mac.IsMulticast()
  ) {
    nPort_t nPort( ofp141::ofp_port_no::OFPP_ALL ); // all but ingress (pg 15)
  }
  else {
    mapMac_t::iterator iter = m_mapMac.find( mac );
    if ( m_mapMac.end() == iter ) {
      //throw std::runtime_error( "Bridge::Lookup: no address found" );
      // default to OFPP_ANY
    }
    else {
      nPort = iter->second.m_inPort;
    }
  }
  return nPort;
}

void Bridge::Forward( ofport_t ofp_ingress, idVlan_t vlan,
                      const MacAddress& macSrc, const MacAddress& macDst,
                      uint8_t* pPacket, size_t nOctets
) {

  bool bSomethingOdd( false );

  bSomethingOdd |= macSrc.IsBroadcast();
  bSomethingOdd |= macSrc.IsMulticast();

  if ( bSomethingOdd ) {
    std::cout
      << "bridge::forward: src is broadcast or multicast, ignoring"
      << std::endl;
  }
  else {

    mapMac_t::iterator iterMapMacSrc = m_mapMac.find( macSrc );
    if ( m_mapMac.end() == iterMapMacSrc ) {
      std::cout
        << "bridge:;forward: src mac is not in lookup, ignoring, should have already been set"
        << std::endl;
    }
    else {

      // look up source port in mapInterface
      //    if vlan is 0, then use as acccess port and lookup tag
      //    if vlan is not zero, confirm vlan belongs as a port (trunk or native), and forward based upon trunk

      mapInterface_t::iterator iterInterface = m_mapInterface.find( ofp_ingress );
      if ( m_mapInterface.end() == iterInterface ) {
        std::cout
          << "bridge::forward - couldn't find inbound interface " << ofp_ingress
          << std::endl;
      }
      else {

        interface_t& interface( iterInterface->second );

        bool bSrcAccess( false );

        if ( ( 0 == vlan ) && ( VlanMode::access == interface.eVlanMode ) ) {
          vlan = interface.tag; // associate vlan with access port
          bSrcAccess = true;
        }
        if ( ( 0 == vlan ) && ( VlanMode::native_tagged == interface.eVlanMode ) ) {
          vlan = interface.tag;
          bSrcAccess = true;
        }

        assert( 0 != vlan ); // not sure what other conditions we are going to have for now

        bool bBroadcast( false );

        bBroadcast |= macDst.IsBroadcast(); // probably redundant comparison, given map lookup below
        bBroadcast |= macDst.IsMulticast(); // probably redundant comparison, given map lookup below

        mapMac_t::iterator iterMapMacDst = m_mapMac.find( macDst );
        bBroadcast |= m_mapMac.end() == iterMapMacDst;

        if ( bBroadcast ) {
          // route via group
          std::cout
            << "bridge::forward broadcast from " << ofp_ingress
            << ", to vlan " << vlan
            << ", on group " << vlan + ( bSrcAccess ? 10000 : 20000 )
            << std::endl;
          vByte_t v = std::move( m_fAcquireBuffer() );
          v.clear();
          auto*  pOut = ::Append<codec::ofp_packet_out::ofp_packet_out_>( v );
          pOut->initv2( ofp_ingress );
          auto* pGroup = ::Append<ofp141::ofp_action_group>( v );
          pGroup->type = ofp141::ofp_action_type::OFPAT_GROUP;
          pGroup->len  = sizeof( ofp141::ofp_action_group );
          pGroup->group_id = vlan + ( bSrcAccess ? 10000 : 20000 );
          pOut->actions_len = pGroup->len; // simple way for now
          vByte_t::size_type size = v.size();
          v.resize( v.size() + nOctets );
          auto* pAppend = v.data() + size;
          std::memcpy( pAppend, pPacket, nOctets );
          pOut->header.length = v.size();
          m_fTransmitBuffer( std::move( v ) );
        }
        else {
          // install rules into table and route via tables
        }
      }


    }
  }


}

void Bridge::UpdateInterface( const interface_t& interface_ ) {

  std::cout << "Bridge::UpdateInterface " << interface_.tag << "," << interface_.ofport << "," << interface_.ifindex << std::endl;

  // 0xfffe seems to match the bridge, can be multiple bridges, same ofport, different ifindex
  if ( ( ofp141::ofp_port_no::OFPP_MAX >= interface_.ofport ) && ( 0xfffe != interface_.ofport ) ) {
    std::unique_lock<std::mutex> lock( m_mutex );
    // is ofport==0 a valid value?  -- no, need to find the reference again
    mapInterface_t::iterator iterMapInterface = m_mapInterface.find( interface_.ofport );
    if ( m_mapInterface.end() == iterMapInterface ) {
      iterMapInterface = m_mapInterface.insert( m_mapInterface.begin(), mapInterface_t::value_type( interface_.ofport, interface_ ) );
    }
    else {
      iterMapInterface->second = interface_;
      // TODO:  be a bit more subtle, check for changes one by one.
      //        ie, delete ofport from access, trunk, global
      //          therefore, may need a PortToVlan mapping?
      //             and may need to delete associated vlan/groups as a consequence of complete deletion?
    }

    interface_t& interface( iterMapInterface->second );

    auto fAddAccess = [this](interface_t& interface){
      mapVlanToPort_t::iterator iterMapVlanToPort;
      iterMapVlanToPort = m_mapVlanToPort.find( interface.tag );
      if ( m_mapVlanToPort.end() == iterMapVlanToPort ) {
        iterMapVlanToPort = m_mapVlanToPort.insert( m_mapVlanToPort.begin(), mapVlanToPort_t::value_type( interface.tag, VlanToPort_t() ) );
      }
      iterMapVlanToPort->second.setPortAccess.insert( interface.ofport );
      iterMapVlanToPort->second.bGroupNeedsUpdate = true;
    };

    auto fAddTrunk = [this](interface_t& interface){
      if ( interface.setTrunk.empty() ) {
        m_setPortWithAllVlans.insert( interface.ofport );
      }
      else {
        mapVlanToPort_t::iterator iterMapVlanToPort;
        for ( auto vlan: interface.setTrunk ) {
          iterMapVlanToPort = m_mapVlanToPort.find( vlan );
          if ( m_mapVlanToPort.end() == iterMapVlanToPort ) {
            iterMapVlanToPort = m_mapVlanToPort.insert( m_mapVlanToPort.begin(), mapVlanToPort_t::value_type( vlan, VlanToPort_t() ) );
          }
          iterMapVlanToPort->second.setPortTrunk.insert( interface.ofport );
          iterMapVlanToPort->second.bGroupNeedsUpdate = true;
        }
      }
    };

    switch ( interface.eVlanMode ) {
      case VlanMode::access:
        assert( 0 != interface.tag );
        fAddAccess( interface );
        break;
      case VlanMode::trunk:
        //assert( !interface.setTrunk.empty() ); // can't do this as all vlans might be acceptable
        fAddTrunk( interface );
        break;
      case VlanMode::native_tagged:
        assert( 0 != interface.tag );
        fAddAccess( interface );
        fAddTrunk( interface );
        break;
      case VlanMode::dot1q_tunnel:
        assert( 0 );
        break;
      case VlanMode::native_untagged:
        assert( 0 );
        break;
    }

    // TODO: on startup, will need to delete or sync up groups already existing in switch

    //std::cout << "** Bridge::m_bRulesInjectionActive test" << std::endl;

    if ( m_bRulesInjectionActive ) {

      //std::cout << "** Bridge::m_bRulesInjectionActive passed" << std::endl;

      BuildGroups();

    }
  }

}

void Bridge::DelInterface( ofport_t ) {
}

void Bridge::UpdateState( ofport_t, OpState admin_state, OpState link_state ) {
}

void Bridge::StartRulesInjection( fAcquireBuffer_t fAcquireBuffer, fTransmitBuffer_t fTransmitBuffer ) {

  //std::cout << "** Bridge::m_bRulesInjectionActive locking" << std::endl;

  std::unique_lock<std::mutex> lock( m_mutex );

  assert( nullptr != fAcquireBuffer );
  m_fAcquireBuffer =  std::move( fAcquireBuffer );
  assert( nullptr != fTransmitBuffer );
  m_fTransmitBuffer = std::move( fTransmitBuffer );

  //std::cout << "** Bridge::m_bRulesInjectionActive to be set" << std::endl;

  m_bRulesInjectionActive = true;

  //std::cout << "** Bridge::m_bRulesInjectionActive is set" << std::endl;

  // TODO: send what we know
  BuildGroups();
}

void Bridge::BuildGroups() {

  struct BuildGroup {
    enum op { pass, push, pop };
    vByte_t v;
    codec::ofp_group_mod::ofp_group_mod_* pMod;
    BuildGroup( vByte_t v_ ): pMod( nullptr), v( std::move( v_ ) ) {}
    void AddCommand( ofp141::ofp_group_mod_command cmd, Bridge::idGroup_t idGroup ) {
      //std::cout << "BuildGroup::AddCommand" << std::endl;
      v.clear();
      pMod = ::Append<codec::ofp_group_mod::ofp_group_mod_>( v );
      pMod->init( cmd, idGroup );
      //std::cout << "BuildGroup::AddCommand: " << pMod->header.length << std::endl;
    }
    void AddOutput( op op_, Bridge::idVlan_t idVlan, Bridge::ofport_t ofport ) {

      //std::cout << "BuildGroup::AddOutput" << std::endl;

      size_t sizeStarting = v.size();

      auto pBucket = ::Append<codec::ofp_group_mod::ofp_bucket_>( v );
      pBucket->init();

      switch ( op_ ) {
        case op::pass:
          // nothing to do
          break;
        case op::pop: {
          auto pAction = ::Append<codec::ofp_flow_mod::ofp_action_pop_vlan_>( v );
          pAction->init();
          }
          break;
        case op::push:
          auto pActionPushVlan = ::Append<codec::ofp_flow_mod::ofp_action_push_vlan_>( v );
          pActionPushVlan->init( 0x8100 );
          auto pActionSetField = ::Append<codec::ofp_flow_mod::ofp_action_set_field_vlan_id_>( v );
          pActionSetField->init( idVlan );
          break;
      }

      auto pAction = ::Append<codec::ofp_flow_mod::ofp_action_output_>( v );
      pAction->init( ofport );
      pAction->max_len = 0;
      //std::cout << "BuildGroup::pAction port " << pAction->port << std::endl;

      pBucket->len = v.size() - sizeStarting;
      pMod->header.length = v.size();
      //std::cout << "BuildGroup::AddOutput: " << pMod->header.length << std::endl;
    }
  };

  for ( auto& entry: m_mapVlanToPort ) {
    idVlan_t idVlan( entry.first );
    assert( 0 < idVlan );
    VlanToPort_t& v2p( entry.second );

    //std::cout << "** BuildGroup: vlan " << idVlan << std::endl;

    if ( v2p.bGroupNeedsUpdate ) {

      // TODO: need two passes: 1) for IN access vlan, 2) for IN trunk vlan
      //   no... single pass, but build group based upon whether inbound is access or trunk
      //         and don't emit if the group has no buckets

      //std::cout << "** BuildGroup: vlan " << idVlan << " update " << std::endl;

      // build group for idVlan
      BuildGroup groupAccess( std::move( m_fAcquireBuffer() ) ); // in_port is access, build outports
      BuildGroup groupTrunk(  std::move( m_fAcquireBuffer() ) ); // in_port is trunk,  build outports

      if ( v2p.bGroupAdded ) {
        //pMod->init( ofp141::ofp_group_mod_command::OFPGC_MODIFY, 10000 + idVlan );
        groupAccess.AddCommand( ofp141::ofp_group_mod_command::OFPGC_MODIFY, 10000 + idVlan );
        groupTrunk.AddCommand(  ofp141::ofp_group_mod_command::OFPGC_MODIFY, 20000 + idVlan );
      }
      else {
        //pMod->init( ofp141::ofp_group_mod_command::OFPGC_ADD, 10000 + idVlan );
        groupAccess.AddCommand( ofp141::ofp_group_mod_command::OFPGC_ADD, 10000 + idVlan );
        groupTrunk.AddCommand(  ofp141::ofp_group_mod_command::OFPGC_ADD, 20000 + idVlan );
      }

      // add buckets for access
      if ( !v2p.setPortAccess.empty() ) {
        for ( auto ofport: v2p.setPortAccess ) {
          groupAccess.AddOutput( BuildGroup::op::pass, idVlan, ofport ); // access to access
          groupTrunk.AddOutput(  BuildGroup::op::pop,  idVlan, ofport );  // trunk to access
        }
      }

      // add buckets for trunk
      if ( !v2p.setPortTrunk.empty() ) {
        for ( auto ofport: v2p.setPortTrunk ) {
          groupAccess.AddOutput( BuildGroup::op::push, idVlan, ofport ); // access to trunk
          groupTrunk.AddOutput(  BuildGroup::op::pass, idVlan, ofport ); // trunk to trunk
        }
      }

      // add buckets for trunk-all
      if ( !m_setPortWithAllVlans.empty() ) {
        for ( auto ofport: m_setPortWithAllVlans ) {
          groupAccess.AddOutput( BuildGroup::op::push, idVlan, ofport ); // access to trunk
          groupTrunk.AddOutput(  BuildGroup::op::pass, idVlan, ofport ); // trunk to trunk
        }
      }

      v2p.bGroupAdded = true;
      v2p.bGroupNeedsUpdate = false;

      assert( 0 != groupAccess.v.size() );
      assert( 0 != groupTrunk.v.size() );

      //std::cout << "** BuildGroup: vlan " << idVlan << " queue access " << groupAccess.v.size() << std::endl;
      m_fTransmitBuffer( std::move( groupAccess.v ) );
      //std::cout << "** BuildGroup: vlan " << idVlan << " queue trunk " << groupTrunk.v.size() << std::endl;
      m_fTransmitBuffer( std::move( groupTrunk.v ) );
    }

  }

  // build group to trasnmit to other trunk-all ports
  if ( !m_setPortWithAllVlans.empty() ) {
    if ( 1 < m_setPortWithAllVlans.size() ) {

      //std::cout << "** BuildGroup: trunk-all" << std::endl;

      BuildGroup groupTrunkAll( std::move( m_fAcquireBuffer() ) ); // in_port is trunk-all, build outports
      if ( m_bGroupTrunkAllAdded ) {
        groupTrunkAll.AddCommand( ofp141::ofp_group_mod_command::OFPGC_MODIFY, 20000 );
      }
      else {
        groupTrunkAll.AddCommand( ofp141::ofp_group_mod_command::OFPGC_ADD, 20000 );
        m_bGroupTrunkAllAdded = true;
      }
      // build bucket for trunk-all ports
      for ( auto ofport: m_setPortWithAllVlans ) {
        groupTrunkAll.AddOutput( BuildGroup::op::pass, 0, ofport ); // 0 vlan is ignored with pass
      }

      assert( 0 != groupTrunkAll.v.size() );

      //std::cout << "** BuildGroup: m_fTransmitBuffer " << " queue trunk all " << groupTrunkAll.v.size() << std::endl;
      m_fTransmitBuffer( std::move( groupTrunkAll.v ) );
    }
  }

}