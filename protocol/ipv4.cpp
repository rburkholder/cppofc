/*
 * File:   ipv4.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on November 7, 2018, 7:57 PM
 */

#include "ipv4/address.h"

#include "ipv4.h"

namespace protocol {
namespace ipv4 {

// ** Header

Header::Header( const Header_& header ): m_header( header ) {
}

Header::~Header() {}

bool Header_::Validate() {
  return
    ( 4 == version() ) &&
    ( 5 <= ihl() )
    ;
}

std::ostream& Header::Emit( std::ostream& stream ) const {

  address src( m_header.source_ip );
  address dst( m_header.destination_ip );

  stream
    << "protocol=" << (uint16_t) m_header.protocol
    << ",ihl=" << (uint16_t) ( m_header.ihl() ) // should be 5 (20 bytes without options)
    << ",src_ip=" << src
    << ",dst_ip=" << dst
    << std::hex
    << ",qos=0x" << (uint16_t) m_header.qos
    << std::dec
    << ",df=" << m_header.df()
    << ",mf=" << m_header.mf()
    << ",ttl=" << (uint16_t) m_header.ttl
    << ",len=" << m_header.length
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
  stream << "ipv4: " << header;
  return stream;
}

} // namespace ipv4
} // namespace protocol
