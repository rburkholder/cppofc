/* 
 * File:   ethernet.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 * 
 * Created on June 12, 2017, 4:53 PM
 */

#include "ethernet.h"

namespace protocol {

Ethernet::Ethernet( uint8_t& rOctets ): m_rOctets( rOctets ) {
}

Ethernet::~Ethernet( ) {
}

std::ostream& operator<<( std::ostream& stream, const protocol::Ethernet& ethernet ) {
  ethernet.Emit( stream );
  return stream;
}

} // namespace protocol

// m_etherType:
//  <= 1500 sizo of payload in octets
//  >=1536 used as EtherType
//    minimum payload is 46 octets
//  FrameType     EtherType   PayloadStart
//  EthernetII    >=1536       any
//  novell 802.3  <=1500      0xFFFF older frame type
//  802.2 LLC     <=1500      other
//  802.2 SNAP    <=1500      0xAAAA


