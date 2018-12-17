/*
 * File:   arp.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on 2017/09/20, 4:53 PM
 */

#include "arp.h"

namespace protocol {
namespace ipv4 {
namespace arp {

// ** Header

Header::Header( const Header_& header ): m_header( header ) {
}

Header::~Header() {}

std::ostream& Header::Emit( std::ostream& stream ) const {
  stream
    << std::hex
    << "hw=" << m_header.m_typeHardware
    << ",proto=" << m_header.m_typeProtocol
    << ",op=" << m_header.m_opcode
    << std::dec
    ;
  return stream;
}

std::ostream& operator<<( std::ostream& stream, const Header& header ) {
  return header.Emit( stream );
}

// ** Content

Content::Content() {
}

Content::~Content() {}

void Content::Init( Header_& header ) {
  m_lenHardware = header.m_lenHardware;
  m_lenProtocol = header.m_lenProtocol;
  m_addrHardwareSender = new( &header.m_data[0] ) uint8_t;
  m_addrProtocolSender = new( m_addrHardwareSender + m_lenHardware ) uint8_t;
  m_addrHardwareTarget = new( m_addrProtocolSender + m_lenProtocol ) uint8_t;
  m_addrProtocolTarget = new( m_addrHardwareTarget + m_lenHardware ) uint8_t;
}

std::ostream& Content::Emit( std::ostream& stream ) const {
  stream
    << ",hs=" << HexDump<uint8_t*>( m_addrHardwareSender, m_addrHardwareSender + m_lenHardware )
    << ",ps=" << HexDump<uint8_t*>( m_addrProtocolSender, m_addrProtocolSender + m_lenProtocol )
    << ",ht=" << HexDump<uint8_t*>( m_addrHardwareTarget, m_addrHardwareTarget + m_lenHardware )
    << ",pt=" << HexDump<uint8_t*>( m_addrProtocolTarget, m_addrProtocolTarget + m_lenProtocol )
    ;
  return stream;
}

std::ostream& operator<<( std::ostream& stream, const Content& content ) {
  return content.Emit( stream );
}

// ** Packet

Packet::Packet( uint8_t& rOctets ) {
  m_pHeader_ = new ( &rOctets ) Header_;
  m_Content.Init( *m_pHeader_ );
}

Packet::~Packet() {
}

std::ostream& operator<<( std::ostream& stream, const Packet& packet ) {
  return packet.Emit( stream );
}

// ** IPv4

ethernet::ethernet( uint8_t& rOctets )
: m_ipv4( new ( &rOctets ) ethernet_t )
{
  Header_& header( m_ipv4->header );
  assert(      1 == header.m_typeHardware );
  assert( 0x0800 == header.m_typeProtocol );
  assert(      6 == header.m_lenHardware );
  assert(      4 == header.m_lenProtocol );
  //assert(    ( 1 == header.m_opcode ) || ( 2 == header.m_opcode ) );
}

ethernet::~ethernet() {
}

std::ostream& ethernet::Emit( std::ostream& stream ) const {
  const protocol::ipv4::address ipv4Sender( IPv4Sender() );
  const protocol::ipv4::address ipv4Target( IPv4Target() );
  const protocol::ethernet::address macSender( MacSender() );
  const protocol::ethernet::address macTarget( MacTarget() );
  stream << "arp: sender(" << ipv4Sender << "," << macSender << "),target(" << ipv4Target << "," << macTarget << ")";
  return stream;
}

std::ostream& operator<<( std::ostream& stream, const ethernet& arp ) {
  return arp.Emit( stream );
}

// ** Cache

void Cache::Update( const protocol::ipv4::address& ipv4, const protocol::ethernet::address& mac ) {
  mapIpv4ToMac_t::iterator iterMapIpv4ToMac = m_mapIpv4ToMac.find( ipv4 );
  if ( m_mapIpv4ToMac.end() == iterMapIpv4ToMac ) {
    iterMapIpv4ToMac = m_mapIpv4ToMac.insert( m_mapIpv4ToMac.begin(), mapIpv4ToMac_t::value_type( ipv4, mac ) );
  }
  else {
    if ( iterMapIpv4ToMac->second.IsAllZero() && !mac.IsAllZero() ) {
      iterMapIpv4ToMac->second = mac;
    }
  }
}

void Cache::Update( const ethernet& arp ) {
  {
    const protocol::ipv4::address     ipv4( arp.IPv4Sender() );
    const protocol::ethernet::address mac(  arp.MacSender() );
    Update( ipv4, mac );
  }
  {
    const protocol::ipv4::address     ipv4( arp.IPv4Target() );
    const protocol::ethernet::address mac(  arp.MacTarget() );
    Update( ipv4, mac );
  }
}

} // namespace arp
} // namespace ipv4
} // namespace protocol