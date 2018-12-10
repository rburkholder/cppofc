/*
 * File:   ethernet.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on June 12, 2017, 4:53 PM
 */

#include "ethernet.h"

namespace protocol {
namespace ethernet {

header::header( uint8_t& rOctets ) {
  m_pHeader = new( &rOctets ) header_;
}

header::~header( ) {
}

std::ostream& header::Emit( std::ostream& stream ) const {

  stream
    << "ethernet:"
    << " ethertype=0x" << std::hex << m_pHeader->m_ethertype << std::dec
    << " srcmac=" << HexDump<uint8_t*>( m_pHeader->m_macSrc, m_pHeader->m_macSrc + 6 )
    << " dstmac=" << HexDump<uint8_t*>( m_pHeader->m_macDest, m_pHeader->m_macDest + 6 )
    ;

  return stream;

}

std::ostream& operator<<( std::ostream& stream, const header& header ) {
  header.Emit( stream );
  return stream;
}

} // namespace ethernet
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
