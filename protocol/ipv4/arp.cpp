/*
 * File:   arp.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on 2017/09/20, 4:53 PM
 */

#include "arp.h"

namespace protocol {
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

IPv4_Ether::IPv4_Ether( uint8_t& rOctets )
: m_ipv4( new ( &rOctets ) IPv4_Ether_ )
{
  Header_& header( m_ipv4->header );
  assert(      1 == header.m_typeHardware );
  assert( 0x0800 == header.m_typeProtocol );
  assert(      6 == header.m_lenHardware );
  assert(      4 == header.m_lenProtocol );
  //assert(    ( 1 == header.m_opcode ) || ( 2 == header.m_opcode ) );
}

IPv4_Ether::~IPv4_Ether() {
}

// ** Cache

void Cache::Update( IPv4_Ether& arp ) {
  protocol::ipv4::address ipv4( arp.IPv4Sender() );
  MacAddress mac( arp.MacSender() );
  mapArp_t::iterator iterMapArpSender = m_mapArp.find( ipv4 );
  if ( m_mapArp.end() == iterMapArpSender ) {
    m_mapArp.insert( mapArp_t::value_type( ipv4, mac ) );
  }
}

} // namespace arp
} // namespace protocol