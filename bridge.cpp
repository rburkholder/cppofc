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

Bridge::Bridge( )
: m_bRulesInjectionActive( false )//, m_idGroup_base( 1 )
{
}

Bridge::~Bridge( ) {
}

Bridge::MacStatus Bridge::Update( nPort_t nPort, const mac_t& macSource ) {

  if ( 0 == nPort ) {
    throw std::runtime_error( "Bridge::Update need port > 0" );
  }

  MacStatus status( StatusQuo );
  MacAddress mac( macSource );

  if ( MacAddress::IsMulticast( mac ) ) {
    status = Multicast;
    std::cout
      << "bridge: found source multicast " << macSource
      << " on port " << nPort
      << std::endl;
  }
  else {
    if ( MacAddress::IsBroadcast( mac ) ) {
      status = Broadcast;
      std::cout
        << "bridge: found source broadcast " << macSource
        << " on port " << nPort
        << std::endl;
    }
    else {
      mapMac_t::iterator iter = m_mapMac.find( mac );
      if ( m_mapMac.end() == iter ) { // didn't find mac
        std::pair<MacAddress, MacInfo> pair( mac, MacInfo( nPort ) );
        m_mapMac.insert( pair );
        status = Learned;
        std::cout
          << "bridge: learned mac " << HexDump<const uint8_t*>( macSource, macSource + 6, ':' )
          << " on port " << nPort
          << std::endl;
      }
      else {
        if ( nPort != iter->second.m_inPort ) { // mac moved (check for flap sometime)
          iter->second.m_inPort = nPort;
          status = Moved;
          std::cout
            << "bridge: mac " << HexDump<const uint8_t*>( macSource, macSource + 6, ':' )
            << " moved to port " << nPort
            << std::endl;
        }
      }
    }
  }

  return status;
}

nPort_t Bridge::Lookup( const mac_t& mac_ ) {
  nPort_t nPort( ofp141::ofp_port_no::OFPP_ANY );  // neither ingress nor egress (pg 15)
  MacAddress mac( mac_ );
  if ( MacAddress::IsBroadcast( mac_ )
    || MacAddress::IsMulticast( mac_ )
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

//void Bridge::AddInterface( const interface_t& ) {
//}

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

void Bridge::UpdateInterface( const interface_t& interface_ ) {

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

  if ( m_bRulesInjectionActive ) {

    for ( auto& entry: m_mapVlanToPort ) {
      idVlan_t idVlan( entry.first );
      VlanToPort_t& v2p( entry.second );

      struct BuildGroup {
        enum op { pass, push, pop };
        vByte_t v;
        BuildGroup( vByte_t v_ ): v( std::move( v_ ) ) {}
        void AddCommand( ofp141::ofp_group_mod_command cmd, Bridge::idGroup_t idGroup ) {
          auto pMod = ::Append<codec::ofp_group_mod::ofp_group_mod_>( v );
          pMod->init( cmd, idGroup );
        }
        void AddOutput( op op_, Bridge::idVlan_t idVlan, Bridge::ofport_t ofport ) {

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

          pBucket->len = v.size();
        }
      };


      if ( v2p.bGroupNeedsUpdate ) {

        // TODO: need two passes: 1) for IN access vlan, 2) for IN trunk vlan
        //   no... single pass, but build group based upon whether inbound is access or trunk
        //         and don't emit if the group has no buckets

        // build group for idVlan
        BuildGroup groupAccess( std::move( m_fAcquireBuffer() ) );
        BuildGroup groupTrunk(  std::move( m_fAcquireBuffer() ) );

        //auto pMod = Append<codec::ofp_group_mod::ofp_group_mod_>( v, sizePacket );

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

            //auto pBucket = Append<codec::ofp_group_mod::ofp_bucket_>( v, sizePacket );
            //pBucket->init();

            //auto pAction = Append<codec::ofp_flow_mod::ofp_action_output_>( v, sizePacket );
            //pAction->init( ofport );

            //pBucket->len += pAction->len;

          }
        }

        // add buckets for trunk
        if ( !v2p.setPortTrunk.empty() ) {
        }

        // add buckets for trunk-all
        if ( !m_setPortWithAllVlans.empty() ) {
        }

        // build group for trunk
        if ( !v2p.setPortTrunk.empty() || !m_setPortWithAllVlans.empty() ) {
          // add group for trunk
          if ( !v2p.setPortTrunk.empty() ) {
            // build bucket for targeted trunk ports
          }
          if ( !m_setPortWithAllVlans.empty() ) {
            // build bucket for trunk-all ports
            for ( auto ofport: m_setPortWithAllVlans ) {
              // vlans need to go to trunk-all ports
            }
          }
        }

        v2p.bGroupAdded = true;
        v2p.bGroupNeedsUpdate = false;

        //pMod->header.length = sizePacket;

        //m_fTransmitBuffer( std::move( v ) );
      }

    }

    // build group to trasnmit to other trunk-all ports
    // use idGroup 0 for base of this group
    // TODO: use group forwarding to use this from above access and trunk port groups
    if ( !m_setPortWithAllVlans.empty() ) {
      // TODO: build only if two or more entries
      // add group for trunk-all port
      // build bucket for trunk-all ports
      for ( auto ofport: m_setPortWithAllVlans ) {
        // vlans need to go to trunk-all ports
      }
    }

  }


}

void Bridge::DelInterface( ofport_t ) {
}

void Bridge::UpdateState( ofport_t, OpState admin_state, OpState link_state ) {
}

void Bridge::StartRulesInjection( fAcquireBuffer_t fAcquireBuffer, fTransmitBuffer_t fTransmitBuffer ) {

  std::unique_lock<std::mutex> lock( m_mutex );

  assert( nullptr != fAcquireBuffer );
  m_fAcquireBuffer =  std::move( fAcquireBuffer );
  assert( nullptr != fTransmitBuffer );
  m_fTransmitBuffer = std::move( fTransmitBuffer );

  m_bRulesInjectionActive = true;

  // TODO: send what we know
}