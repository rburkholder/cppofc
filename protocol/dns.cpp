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

std::ostream& operator<<( std::ostream& os, const uint8_t* p ) {
  os
    << (uint16_t) *(p + 0) << "."
    << (uint16_t) *(p + 1) << "."
    << (uint16_t) *(p + 2) << "."
    << (uint16_t) *(p + 3)
    ;
  return os;
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
  m_pPacket = &rOctets;
  m_pHeader = new ( &rOctets ) header_;
  m_pData = &rOctets + sizeof( header_ );
  //m_Content.Init( *m_pHeader_ );
}

Packet::~Packet() {
}

// TODO: prevent recursion in pointer
// TODO: prevent pointer from extending beyond packet
uint8_t* Packet::DecodeName( uint8_t* p, std::string& name ) const {
  //assert( 0 != p ); // is zero length allowed? is this what a root request is?
  // assert( 0 == name.size();
  if ( 0 < ( 0xc0 & *p ) ) assert( 0xc0 == ( 0xc0 & *p ) );  // 0x00 and 0xc0 only allowed
  if ( 0xc0 == ( 0xc0 & *p ) ) {
    auto* pOffset = new ( p ) endian::big_uint16_t;
    uint16_t offset( *pOffset );
    DecodeName( m_pPacket + ( 0x3fff & offset ), name );
    p += 2;
  }
  else {
    if ( 0 != *p ) {
      std::string label( p + 1, p + *p + 1 );
      if ( 0 < name.size() ) name += ".";
      name += label;
      p += ( *p + 1 );
      p = DecodeName( p, name );
    }
    else {
      p++;
    }
  }
  return p;
}

uint8_t* Packet::DecodeQuestion( uint8_t* p, question_& q ) const {
  p = DecodeName( p, q.m_name );
  endian::big_uint16_t* pQType = new ( p ) endian::big_uint16_t;
  q.m_qtype = *pQType;
  p += 2;
  endian::big_uint16_t* pQClass = new ( p ) endian::big_uint16_t;
  q.m_qclass = *pQClass;
  p+= 2;
  return p;
}

uint8_t* Packet::DecodeResourceRecord( uint8_t* p, rr_& rr ) const {
  p = DecodeName( p, rr.m_name );
  endian::big_uint16_t* pType = new ( p ) endian::big_uint16_t;
  rr.m_type = *pType;
  p += 2;
  endian::big_uint16_t* pClass = new ( p ) endian::big_uint16_t;
  rr.m_class = *pClass;
  p += 2;
  endian::big_uint32_t* pTTL = new ( p ) endian::big_uint32_t;
  rr.m_ttl = *pTTL;
  p += 4;
  endian::big_uint16_t* pRdLength = new( p ) endian::big_uint16_t;
  rr.m_rdlen = *pRdLength;
  p += 2;
  rr.m_prd = p;
  p += rr.m_rdlen;
  return p;
}

std::ostream& operator<<( std::ostream& stream, const Packet& packet ) {
  Header header( *packet.m_pHeader );
  stream << "dns: " << header;
  uint8_t* p = packet.m_pData;
  for ( uint16_t ix = 0; ix < packet.m_pHeader->qdcount; ix++ ) {
    question_ q;
    p = packet.DecodeQuestion( p, q );
    stream << ",qstn("
           << q.m_name
           << ",qtype=" << q.m_qtype
           << ",qclass=" << q.m_qclass
           << ")";
  }
  for ( uint16_t ix = 0; ix < packet.m_pHeader->ancount; ix++ ) {
    rr_ rr;
    p = packet.DecodeResourceRecord( p, rr );
    stream << ",ansr("
           << rr.m_name
           << ",type=" << rr.m_type
           << ",class=" << rr.m_class
           << ",ttl=" << rr.m_ttl
           << ",len=" << rr.m_rdlen;
    if ( rr_type::A == rr.m_type ) stream << ",ip=" << rr.m_prd;
    stream << ")";
  }
  for ( uint16_t ix = 0; ix < packet.m_pHeader->nscount; ix++ ) {
    rr_ rr;
    p = packet.DecodeResourceRecord( p, rr );
    stream << ",nsrr("
           << rr.m_name
           << ",type=" << rr.m_type
           << ",class=" << rr.m_class
           << ",ttl=" << rr.m_ttl
           << ",len=" << rr.m_rdlen;
    if ( rr_type::A == rr.m_type ) stream << ",ip=" << rr.m_prd;
    stream << ")";
  }
  for ( uint16_t ix = 0; ix < packet.m_pHeader->arcount; ix++ ) {
    rr_ rr;
    p = packet.DecodeResourceRecord( p, rr );
    stream << ",addrr("
           << rr.m_name
           << ",type=" << rr.m_type
           << ",class=" << rr.m_class
           << ",ttl=" << rr.m_ttl
           << ",len=" << rr.m_rdlen;
    if ( rr_type::A == rr.m_type ) stream << ",ip=" << rr.m_prd;
    stream << ")";
  }

  return stream;
}

} // namespace dns
} // namespace protocol
