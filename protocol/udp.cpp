/* 
 * File:   udp.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 * 
 * Created on November 8, 2018, 1:26 PM
 */

#include "udp.h"

namespace protocol {
namespace udp {

// ** Header

Header::Header( const Header_& header ): m_header( header ) {
}

Header::~Header() {}

std::ostream& Header::Emit( std::ostream& stream ) const {
  stream 
    <<  "src_port=" << m_header.src_port
    << ",dst_port=" << m_header.dst_port
    << ",length="   << m_header.length
    << ",checksum=" << m_header.checksum
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
  Header header( *packet.m_pHeader_ );
  stream << "udp=" << header;
  return stream;
}

} // namespace udp
} // namespace protocol
