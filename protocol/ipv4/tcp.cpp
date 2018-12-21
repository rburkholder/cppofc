/*
 * File:   tcp.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on November 8, 2018, 3:30 PM
 */

#include "../../hexdump.h"

#include "tcp.h"

namespace protocol {
namespace tcp {

// ** Header

Header::Header( const header_& header ): m_header( header ) {
}

Header::~Header() {}

std::ostream& Header::Emit( std::ostream& stream ) const {
  uint16_t offset = m_header.offset();
  stream
    <<  "src_port=" << m_header.src_port
    << ",dst_port=" << m_header.dst_port
    << ",seq="      << m_header.sequence_num
    << ",ack="      << m_header.ack() << "," << m_header.acknowledgement_num
//    << ",checksum=" << m_header.checksum
    << ",syn="      << m_header.syn()
    << ",rst="      << m_header.rst()
    << ",fin="      << m_header.fin()
    << ",psh="      << m_header.psh()
    << ",urg="      << m_header.urg()
    << ",ns="       << m_header.ns()
    << ",cwr="      << m_header.cwr()
    << ",ece="      << m_header.ece()
    << ",offset="   << offset
    ;

  if ( 5 < offset ) {
    // TODO: ensure no overflow
    const uint8_t* pb = m_header.options;
    const uint8_t* pe = pb + ( ( offset - 5 ) * 4 );
    stream << ",opt[";
    while ( ( 0 != *pb ) && ( pb < pe ) ) {
      uint16_t value( *pb++ );
      uint16_t size( 0 );
      if ( 1 != value ) size = *pb++;
      switch ( value ) {
        case 1:
          stream << "nop";
          break;
        case 2:
          stream << "mss=";
          assert( 4 == size );
          {
            uint16_t mss = *pb++ << 8;
            mss += *pb++;
            stream << mss;
          }
          break;
        case 3:
          stream << "wscale=";
          assert( 3 == size );
          stream << (uint16_t) *pb++;
          break;
        case 4:
          stream << "sack1";
          assert( 2 == size );
          break;
        case 5:
          stream << "sack2=";
          assert( 2 < size );
          {
            uint8_t cnt( size - 2 );
            stream << HexDump<const uint8_t*>( pb, pb + cnt );
            pb += cnt;
          }
          break;
        case 8:
          stream << "ts=";
          assert( 10 == size );
          {
            uint32_t t1 = *pb++;
            t1 = t1 << 8;
            t1 += *pb++;
            t1 = t1 << 8;
            t1 += *pb++;
            t1 = t1 << 8;
            t1 += *pb++;

            uint32_t t2 = *pb++;
            t2 = t2 << 8;
            t2 += *pb++;
            t2 = t2 << 8;
            t2 += *pb++;
            t2 = t2 << 8;
            t2 += *pb++;

            stream << t1 << "," << t2;
          }
          break;
        default:
          stream << value;
          pb += ( size - 2 );
          break;
      }
      if ( pb < pe ) stream << " ";
    }
    stream << "]";
  }

  return stream;
}

std::ostream& operator<<( std::ostream& stream, const Header& header ) {
  return header.Emit( stream );
}

// ** Packet

Packet::Packet( uint8_t& rOctets, uint16_t len ) {
  m_pHeader_ = new ( &rOctets ) header_;
  //m_Content.Init( *m_pHeader_ );
}

Packet::~Packet() {
}

std::ostream& operator<<( std::ostream& stream, const Packet& packet ) {
  Header header( *packet.m_pHeader_ );
  stream << "tcp=" << header;
  return stream;
}

} // namespace tcp
} // namespace protocol

// TCP Illustrated v1, e2, pg 671 has state machine diagram