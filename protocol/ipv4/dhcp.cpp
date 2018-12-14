/*
 * File:   dhcp.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on November 13, 2018, 7:50 PM
 */

#include <sstream>

#include "../../hexdump.h"

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

Header::Header( Header_& header ): m_header( header ) {
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
    << ",mac=" << std::hex
      << (uint16_t)m_header.chaddr[0] << ":"
      << (uint16_t)m_header.chaddr[1] << ":"
      << (uint16_t)m_header.chaddr[2] << ":"
      << (uint16_t)m_header.chaddr[3] << ":"
      << (uint16_t)m_header.chaddr[4] << ":"
      << (uint16_t)m_header.chaddr[5]
    << std::dec
    << ",name='" << m_header.szname << "'"
    << ",file='" << m_header.szfile << "'"
    << ",cookie=" << HexDump<const uint8_t*>( m_header.magic_cookie, m_header.magic_cookie + 4 )
    //<< ",option=" << m_header.options[0]
    ;

  // TODO: need to test for magic cookie prior to performing decode below
  uint8_t* p = m_header.options;
  while ( 255 != *p ) { // TODO: issue with DoS here without packet length for comparison
    uint8_t id = *p; p++;
    if ( 0 != *p ) {
      uint8_t len = *p; p++;
      switch ( id ) {
        case 1:
          std::cout << ",subnet=" << HexDump<const uint8_t*>( p, p + len );
          break;
        case 3:
          std::cout << ",router=" << HexDump<const uint8_t*>( p, p + len );
          break;
        case 6:
          std::cout << ",dns=" << HexDump<const uint8_t*>( p, p + len );
          break;
        case 12: { // TODO: check for 0 term.
            std::cout << ",host_name=";
            uint8_t* ix = p;
            for ( uint8_t cnt = len; 0 != cnt; cnt-- ) {
              std::cout << *ix;
              ix++;
            }
          }
          break;
        case 15: { // TODO: check for 0 term.
            std::cout << ",domain_name=";
            uint8_t* ix = p;
            for ( uint8_t cnt = len; 0 != cnt; cnt-- ) {
              std::cout << *ix;
              ix++;
            }
          }
          break;
        case 26: {
          auto* mtu = new( p ) endian::big_int16_t;
          std::cout << ",mtu=" << *mtu;
          }
          break;
        case 28:
          std::cout << ",bcast=" << HexDump<const uint8_t*>( p, p + len );
          break;
        case 33:
          std::cout << ",static_route=" << HexDump<const uint8_t*>( p, p + len );
          break;
        case 35: {
          auto* to = new( p ) endian::big_int32_t;
          std::cout << ",arp_timeout=" << *to;
          }
          break;
        case 42:
          std::cout << ",ntp=" << HexDump<const uint8_t*>( p, p + len );
          break;
        case 44:
          std::cout << ",NBNS=" << HexDump<const uint8_t*>( p, p + len );
          break;
        case 45:
          std::cout << ",NBDD=" << HexDump<const uint8_t*>( p, p + len );
          break;
        case 46:
          std::cout << ",NBNodeType=0x" << std::hex << *p << std::dec;
          break;
        case 47:
          std::cout << ",NBScope=" << HexDump<const uint8_t*>( p, p + len );
          break;
        case 50:
          std::cout << ",requested_ip=" << HexDump<const uint8_t*>( p, p + len );
          break;
        case 51: {
          auto* lt = new( p ) endian::big_int32_t;
          std::cout << ",lease=" << *lt;
          }
          break;
        case 53:
          std::cout << ",message=";
          switch ( *p ) {
            case 1:
              std::cout << "DHCPDISCOVER";
              break;
            case 2:
              std::cout << "DHCPOFFER";
              break;
            case 3:
              std::cout << "DHCPREQUEST";
              break;
            case 4:
              std::cout << "DHCPDECLINE";
              break;
            case 5:
              std::cout << "DHCPACK";
              break;
            case 6:
              std::cout << "DHCPNAK";
              break;
            case 7:
              std::cout << "DHCPRELEASE";
              break;
            case 8:
              std::cout << "DHCPINFORM";
              break;
            case 9:
              std::cout << "unknown";
              break;
          }
          break;
        case 54:
          std::cout << ",server=" << HexDump<const uint8_t*>( p, p + len );
          break;
        case 55:
          std::cout << ",param_request=" << HexDump<const uint8_t*>( p, p + len );
          break;
        case 57: {
          auto* msg = new( p ) endian::big_int16_t;
          std::cout << ",max_msg_size=" << *msg;
          }
          break;
        case 58: {
          auto* t1 = new( p ) endian::big_int32_t;
          std::cout << ",t1_renew=" << *t1;
          }
          break;
        case 59: {
          auto* t2 = new( p ) endian::big_int32_t;
          std::cout << ",t2_renew=" << *t2;
          }
          break;
        case 61:
          std::cout << ",client_id=" << HexDump<const uint8_t*>( p, p + len );
          break;
        case 69:
          std::cout << ",smtp=" << HexDump<const uint8_t*>( p, p + len );
          break;
        case 70:
          std::cout << ",pop3=" << HexDump<const uint8_t*>( p, p + len );
          break;
        case 71:
          std::cout << ",nntp=" << HexDump<const uint8_t*>( p, p + len );
          break;
        case 74:
          std::cout << ",irc=" << HexDump<const uint8_t*>( p, p + len );
          break;
        default:
          std::cout << ",option " << (uint16_t) id << "=" << HexDump<const uint8_t*>( p, p + len );
          break;
      }

      p += len;
    }
  }

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
