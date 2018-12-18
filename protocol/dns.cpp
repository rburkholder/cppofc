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
  uint16_t flags( m_header.flags );
  stream
    << "id=" << m_header.id
    << ",flags=("
      << ( ( flags & header_flag_qr ) ? "resp" : "qry" )
      ;
  stream << ",op:";
  switch ( ( flags & header_flag_opcode ) >> 11 ) {
    case 0:
      stream << "qry";
      break;
    case 1:
      stream << "inv_qry";
      break;
    case 2:
      stream << "stat";
      break;
    default:
      stream << "rsrvd";
      break;
  };
  stream
    << ",auth:"
    << ( ( flags & header_flag_aa ) ? "yes" : "no" )
    << ",trunc:"
    << ( ( flags & header_flag_tc ) ? "yes" : "no" )
    << ",recurse_desired:"
    << ( ( flags & header_flag_rd ) ? "yes" : "no" )
    << ",recurse_avail:"
    << ( ( flags & header_flag_ra ) ? "yes" : "no" )
    << ",rcode="
    ;
  switch ( flags & header_flag_rcode ) {
    case 0:
      stream << "no_error";
      break;
    case 1:
      stream << "format_error";
      break;
    case 2:
      stream << "server_fail";
      break;
    case 3:
      stream << "name_error";
      break;
    case 4:
      stream << "not_impl";
      break;
    case 5:
      stream << "refused";
      break;
    default:
      stream << "reserved(" << ( flags & header_flag_rcode );
  };
  stream
    << ")"
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
