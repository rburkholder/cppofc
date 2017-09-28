/* 
 * File:   bridge.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 * 
 * Created on September 22, 2017, 6:05 PM
 */

#ifndef BRIDGE_H
#define BRIDGE_H

#include <cstdint>
#include <cstring>
#include <unordered_map>

#include "common.h"

class MacAddress {
public:
  
  MacAddress() {
    std::memset( m_mac, 0, sizeof( mac_t ) );
  }
  
  MacAddress( const mac_t& mac ) {
    std::memcpy( m_mac, mac, sizeof( mac_t ) );
  }
  
  MacAddress( const MacAddress& rhs ) {
    std::memcpy( m_mac, rhs.m_mac, sizeof( mac_t ) );
  }
  
  const mac_t& Value() const { return m_mac; }
  
  bool operator==( const mac_t& rhs ) const {
    return 0 == std::memcmp( m_mac, rhs, sizeof( mac_t ) );
  }
  
  bool operator==( const MacAddress& rhs ) const { 
    return 0 == std::memcmp( m_mac, rhs.m_mac, sizeof( mac_t ) );
  }
  
protected:
private:
  mac_t m_mac;
};

// http://en.cppreference.com/w/cpp/utility/hash
namespace std {
  template<> struct hash<MacAddress> {
    typedef MacAddress argument_type;
    typedef std::size_t result_type;
    result_type operator()( const argument_type& mac ) const {
      return std::hash<const mac_t&>{}( mac.Value() );
      }
    };
}


class Bridge {
public:
  
  enum MacStatus { StatusQuo, Broadcast, Learned, Moved }; // add 'Flap' ?
  
  Bridge( );
  virtual ~Bridge( );
  
  MacStatus Update( nPort_t nPort, const mac_t& macSource );
  
private:
  
  struct MacInfo {
    nPort_t m_inPort;
    MacInfo( nPort_t inPort ): m_inPort( inPort ) {}
  };
  
  // change to pimpl for use with MacAddress
  typedef std::unordered_map<MacAddress,MacInfo> mapMac_t;
  
  mapMac_t m_mapMac;

};

#endif /* BRIDGE_H */

