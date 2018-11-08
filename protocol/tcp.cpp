/* 
 * File:   tcp.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 * 
 * Created on November 8, 2018, 3:30 PM
 */

#include "tcp.h"

namespace protocol {
namespace tcp {

// ** Header

Header::Header( const Header_& header ): m_header( header ) {
}

Header::~Header() {}

std::ostream& Header::Emit( std::ostream& stream ) const {
  stream 
    <<  "src_port=" << m_header.src_port
    << ",dst_port=" << m_header.dst_port
    << ",seq="      << m_header.sequence_num
    << ",ack="      << m_header.acknowledgement_num
    << ",checksum=" << m_header.checksum
    << ",syn="      << m_header.syn() 
    << ",ack="      << m_header.ack()
    << ",rst="      << m_header.rst()
    << ",fin="      << m_header.fin()
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
  stream << "tcp=" << header;
  return stream;
}

} // namespace tcp
} // namespace protocol
