/* 
 * File:   dhcp.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 * 
 * Created on November 13, 2018, 7:50 PM
 */

#include <sstream>

#include "dhcp.h"

namespace protocol {
namespace ipv4 {
namespace dhcp {

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

Header::Header( const Header_& header ): m_header( header ) {
}

Header::~Header() {}

std::ostream& Header::Emit( std::ostream& stream ) const {
  stream 
    << "op=" << (uint16_t)m_header.op
    << ",htype=" << (uint16_t)m_header.htype
    << ",hlen=" << (uint16_t)m_header.hlen
    << ",hops=" << (uint16_t)m_header.hops
    << std::hex
    << ",xid=0x" << m_header.xid
    << std::dec
    << ",secs=" << m_header.secs
    << std::hex
    << ",flags=0x" << m_header.flags
    << std::dec
    << ",ciaddr=" << format(m_header.ciaddr)
    << ",yiaddr=" << format(m_header.yiaddr)
    << ",siaddr=" << format(m_header.siaddr)
    << ",giaddr=" << format(m_header.giaddr)
    << "mac=" << std::hex
      << (uint16_t)m_header.chaddr[0] << ":"
      << (uint16_t)m_header.chaddr[1] << ":"
      << (uint16_t)m_header.chaddr[2] << ":"
      << (uint16_t)m_header.chaddr[3] << ":"
      << (uint16_t)m_header.chaddr[4] << ":"
      << (uint16_t)m_header.chaddr[5]
    << std::dec
    << "option=" << m_header.options[0]
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
  stream << "dhcp: " << header;
  return stream;
}

} // namespace dhcp
} // namespace ipv4
} // namespace protocol
