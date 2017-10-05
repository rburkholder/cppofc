/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   mac.cpp
 * Author: sysadmin
 * 
 * Created on October 5, 2017, 7:09 PM
 */

#include "mac.h"

namespace {
static const mac_t broadcast = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
}

bool MacAddress::IsBroadcast( const mac_t& mac ) {
  return 0 == std::memcmp( mac, broadcast, 6 );
}

bool MacAddress::IsBroadcast( const MacAddress& mac) {
  return IsBroadcast( mac.m_mac );
}

MacAddress::MacAddress() {
  std::memset( m_mac, 0, sizeof( mac_t ) );
}

MacAddress::MacAddress( const mac_t& mac ) {
  std::memcpy( m_mac, mac, sizeof( mac_t ) );
}

MacAddress::MacAddress( const MacAddress& rhs ) {
  std::memcpy( m_mac, rhs.m_mac, sizeof( mac_t ) );
}

const mac_t& MacAddress::Value() const { return m_mac; }

bool MacAddress::operator==( const mac_t& rhs ) const {
  return 0 == std::memcmp( m_mac, rhs, sizeof( mac_t ) );
}

bool MacAddress::operator==( const MacAddress& rhs ) const { 
  return 0 == std::memcmp( m_mac, rhs.m_mac, sizeof( mac_t ) );
}
