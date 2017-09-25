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
#include <unordered_map>

#include "common.h"

class MacAddress {
public:
  
  MacAddress() {
    m_mac[0] = 0;
    m_mac[1] = 0;
    m_mac[2] = 0;
    m_mac[3] = 0;
    m_mac[4] = 0;
    m_mac[5] = 0;
  }
  
  MacAddress( const mac_t& mac ) {
    m_mac[0] = mac[0];
    m_mac[1] = mac[1];
    m_mac[2] = mac[2];
    m_mac[3] = mac[3];
    m_mac[4] = mac[4];
    m_mac[5] = mac[5];
  }
  
  MacAddress( const MacAddress& rhs ) {
    m_mac[0] = rhs.m_mac[0];
    m_mac[1] = rhs.m_mac[1];
    m_mac[2] = rhs.m_mac[2];
    m_mac[3] = rhs.m_mac[3];
    m_mac[4] = rhs.m_mac[4];
    m_mac[5] = rhs.m_mac[5];
  }
  
  const mac_t& Value() const { return m_mac; }
  
  bool operator==( const mac_t& rhs ) const {
    return 
      ( m_mac[0] == rhs[0] ) &&
      ( m_mac[1] == rhs[1] ) &&
      ( m_mac[2] == rhs[2] ) &&
      ( m_mac[3] == rhs[3] ) &&
      ( m_mac[4] == rhs[4] ) &&
      ( m_mac[5] == rhs[5] ) 
      ;
  }
  
  bool operator==( const MacAddress& rhs ) const { 
    return 
      ( m_mac[0] == rhs.m_mac[0] ) &&
      ( m_mac[1] == rhs.m_mac[1] ) &&
      ( m_mac[2] == rhs.m_mac[2] ) &&
      ( m_mac[3] == rhs.m_mac[3] ) &&
      ( m_mac[4] == rhs.m_mac[4] ) &&
      ( m_mac[5] == rhs.m_mac[5] ) 
      ;
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
  
  Bridge( );
  virtual ~Bridge( );
  
  void Update( nPort_t nPort, const mac_t& macSource );
  
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

