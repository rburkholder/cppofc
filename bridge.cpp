/* 
 * File:   bridge.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 * 
 * Created on September 22, 2017, 6:05 PM
 */

#include "hexdump.h"

#include "bridge.h"

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
  return status;
}

nPort_t Bridge::Lookup( const mac_t& mac_ ) {
  nPort_t nPort( 0 );
  MacAddress mac( mac_ );
  if ( MacAddress::IsBroadcast( mac_ ) ) {
  }
  else {
    mapMac_t::iterator iter = m_mapMac.find( mac );
    if ( m_mapMac.end() == iter ) {
      throw std::runtime_error( "Bridge::Lookup: no address found" );
    }
    else {
      return iter->second.m_inPort;
    }
  }
  return nPort;
}