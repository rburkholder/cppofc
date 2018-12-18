/*
 * File:   dns.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net *
 *
 * Created on December 14, 2018, 4:06 PM
 */

#include <sstream>

#include "dns.h"

namespace protocol {
namespace dns {

// ** Header
namespace {
std::string format( uint32_t addr ) {
  std::stringstream ss;
  ss
    << (uint16_t)(               addr  >> 24 ) << "."
    << (uint16_t)( (0x00ff0000 & addr) >> 16 ) << "."
    << (uint16_t)( (0x0000ff00 & addr) >>  8 ) << "."
    << (uint16_t)( (0x000000ff & addr)       )
    ;
  return ss.str();
}
}

Header::Header( header_& header ): m_header( header ) {
}

Header::~Header() {}

std::ostream& Header::Emit( std::ostream& stream ) const {
  stream
    << "id=" << m_header.id
    << std::hex
    << ",flags=0x" << m_header.flags
    << std::dec
    << ",ques=" << m_header.qdcount
    << ",answ=" << m_header.ancount
    << ",nsrr=" << m_header.nscount
    << ",addrr=" << m_header.arcount
    ;

  return stream;
}

std::ostream& operator<<( std::ostream& stream, const Header& header ) {
  return header.Emit( stream );
}

// ** Packet

Packet::Packet( uint8_t& rOctets ) {
  m_pHeader_ = new ( &rOctets ) header_;
  //m_Content.Init( *m_pHeader_ );
}

Packet::~Packet() {
}

std::ostream& operator<<( std::ostream& stream, const Packet& packet ) {
  Header header( *packet.m_pHeader_ );
  stream << "dns: " << header;
  return stream;
}

} // namespace dns
} // namespace protocol
