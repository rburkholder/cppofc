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

Bridge::Bridge( )
: m_bRulesInjectionActive( false )
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
  }

  interface_t& interface( iterMapInterface->second );

  auto fAddAccess = [this](interface_t& interface){
    mapVlanToPort_t::iterator iterMapVlanToPort;
    iterMapVlanToPort = m_mapVlanToPort.find( interface.tag );
    if ( m_mapVlanToPort.end() == iterMapVlanToPort ) {
      iterMapVlanToPort = m_mapVlanToPort.insert( m_mapVlanToPort.begin(), mapVlanToPort_t::value_type( interface.tag, vlan_t() ) );
    }
    iterMapVlanToPort->second.setPortAccess.insert( interface.ofport );
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
          iterMapVlanToPort = m_mapVlanToPort.insert( m_mapVlanToPort.begin(), mapVlanToPort_t::value_type( vlan, vlan_t() ) );
        }
        iterMapVlanToPort->second.setPortAccess.insert( interface.ofport );
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

  if ( m_bRulesInjectionActive ) {

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