/* 
 * File:   ipv6.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 * 
 * Created on November 11, 2018, 12:53 PM
 */

#include "ipv6.h"

namespace protocol {
namespace ipv6 {

// ** Header

Header::Header( const Header_& header ): m_header( header ) {
}

Header::~Header() {}

std::ostream& Header::Emit( std::ostream& stream ) const {
  stream 
    << "traffic_class=" << m_header.traffic_class()
    << ",flow_label=" << m_header.flow_label()
    << ",length=" << m_header.payload_length
    << ",next_header=" << (uint16_t) m_header.next_header
    << ",hop_limit=" << (uint16_t) m_header.hop_limit
    ;
  stream << ",sip=" << std::hex << (uint16_t)m_header.ip_src[0];
  for ( int ix = 1; ix <16; ix++ ) stream << ":" << std::hex << (uint16_t)m_header.ip_src[ ix ];
  stream << ",dip=" << std::hex << (uint16_t)m_header.ip_dst[0];
  for ( int ix = 1; ix <16; ix++ ) stream << ":" << std::hex << (uint16_t)m_header.ip_dst[ ix ];
  stream << std::dec;
  return stream;
}

std::ostream& operator<<( std::ostream& stream, const Header& header ) {
  return header.Emit( stream );
}

// ** Packet

Packet::Packet( uint8_t& rOctets ) {
  m_pHeader_ = new ( &rOctets ) Header_;
  //m_Content.Init( *m_pHeader_ );
}

Packet::~Packet() {
}

std::ostream& operator<<( std::ostream& stream, const Packet& packet ) {
  Header header( *packet.m_pHeader_ );
  stream << "ipv6: " << header;
  return stream;
}

} // namespace ipv6
} // namespace protocol
