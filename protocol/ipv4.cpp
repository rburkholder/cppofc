/*
 * File:   ipv4.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on November 7, 2018, 7:57 PM
 */

#include "../hexdump.h"

#include "ipv4/address.h"

#include "ipv4.h"

namespace protocol {
namespace ipv4 {

// ** Header

Header::Header( const header_& header ): m_header( header ) {
}

Header::~Header() {}

bool header_::validate() const {
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
    << ",data_len=" << m_header.data_len()
    ;

  if ( 5 < m_header.ihl() ) {
    stream << HexDump<const uint8_t*>( m_header.options, m_header.options + ( ( m_header.ihl() - 5 ) * 4 ) );
  }

  return stream;
}

bool Header::Validate(uint16_t len) const {
  return m_header.validate() &&  ( m_header.length == len );
}

std::ostream& operator<<( std::ostream& stream, const Header& header ) {
  return header.Emit( stream );
}

// ** Packet

Packet::Packet( uint8_t& rOctets, uint16_t len ) {
  m_pheader = new ( &rOctets ) header_;
  Header header_( *m_pheader );
  //assert( header_.Validate( len ) );
  //m_Content.Init( *m_pHeader_ );
}

Packet::~Packet() {
}

std::ostream& operator<<( std::ostream& stream, const Packet& packet ) {
  Header header_( *packet.m_pheader );
  stream << "ipv4: " << header_;
  return stream;
}

} // namespace ipv4
} // namespace protocol
