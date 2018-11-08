/* 
 * File:   ipv4.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 * 
 * Created on November 7, 2018, 7:57 PM
 */

#include "ipv4.h"

namespace protocol {
namespace ipv4 {

// ** Header

Header::Header( const Header_& header ): m_header( header ) {
}

Header::~Header() {}

std::ostream& Header::Emit( std::ostream& stream ) const {
  stream 
    << "ihl=" << (uint16_t) ( m_header.version_ihl & 0x0f ) // should be 5 (20 bytes without options)
    << std::hex 
    << ",src_ip=" << m_header.source_ip
    << ",dst_ip=" << m_header.destination_ip
    << ",qos=" << (uint16_t) m_header.qos
    << std::dec
    << ",ttl=" << (uint16_t) m_header.ttl
    << ",protocol=" << (uint16_t) m_header.protocol
    ;
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
  return packet.Emit( stream );
}




} // namespace ipv4
} // namespace protocol