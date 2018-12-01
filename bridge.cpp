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

Bridge::Bridge( ) {
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

void Bridge::UpdateInterface( const interface_t& interface ) {
  // is ofport==0 a valid value?
  mapInterface_t::iterator iterMapInterface = m_mapInterface.find( interface.ofport );
  if ( m_mapInterface.end() == iterMapInterface ) {
    iterMapInterface = m_mapInterface.insert( m_mapInterface.begin(), mapInterface_t::value_type( interface.ofport, interface ) );
  }
  else {
    iterMapInterface->second = interface;
    // TODO:  be a bit more subtle, check for changes one by one.
    // TODO:  update local relationships for supporting easy bridge matching.
  }
}

void Bridge::DelInterface( ofport_t ) {
}

void Bridge::UpdateState( ofport_t, OpState admin_state, OpState link_state ) {
}

