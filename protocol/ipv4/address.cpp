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

} // namespace ipv4
} // namespace protocol
