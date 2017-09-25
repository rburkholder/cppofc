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

void Bridge::Update( nPort_t nPort, const mac_t& macSource ) {
  static const mac_t broadcast = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
  MacAddress mac( macSource );
  if ( mac == broadcast ) {
    std::cout 
      << "bridge: found source broadcast " << macSource
      << " on port " << nPort 
      << std::endl;
  }
  else {
    mapMac_t::iterator iter = m_mapMac.find( mac );
    if ( m_mapMac.end() == iter ) {\
      std::pair<MacAddress, MacInfo> pair( mac, MacInfo( nPort ) );
      m_mapMac.insert( pair );
      std::cout 
        << "bridge: learned mac " << HexDump<const uint8_t*>( macSource, macSource + 6, ':' ) 
        << " on port " << nPort
        << std::endl;
    }
    else {
      if ( nPort != iter->second.m_inPort ) {
        iter->second.m_inPort = nPort;
        std::cout 
          << "bridge: mac " << HexDump<const uint8_t*>( macSource, macSource + 6, ':' )
          << " moved to port " << nPort 
          << std::endl;
      }
    }
  }
}