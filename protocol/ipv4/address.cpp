/*
 * File:   address.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on December 9, 2018, 4:54 PM
 */

#include <cstring>

#include "address.h"

namespace protocol {
namespace ipv4 {

std::ostream& operator<<( std::ostream& stream, const protocol::ipv4::address_t& ipv4 ) {
  stream
    << (uint16_t)ipv4[0] << "."
    << (uint16_t)ipv4[1] << "."
    << (uint16_t)ipv4[2] << "."
    << (uint16_t)ipv4[3];
  return stream;
}

address::address( ) {
}

address::address( const address_t& ipv4 ) {
  std::memcpy( m_ipv4, ipv4, sizeof( address_t ) );
}

address::address( const address& rhs ) {
  std::memcpy( m_ipv4, rhs.m_ipv4, sizeof( address_t ) );
}

address::~address( ) {
}

bool address::operator==( const address_t& rhs ) const {
  return 0 == memcmp( m_ipv4, rhs, sizeof( address_t ) );
}

bool address::operator==( const address& rhs ) const {
  return 0 == memcmp( m_ipv4, rhs.m_ipv4, sizeof( address_t ) );
}

std::ostream& address::Emit( std::ostream& stream ) const {
  stream << m_ipv4;
  return stream;

}

std::ostream& operator<<( std::ostream& stream, const address& ipv4 ) {
  return ipv4.Emit( stream );
}

} // namespace ipv4
} // namespace protocol
