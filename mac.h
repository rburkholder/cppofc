/* 
 * File:   mac.h
 * Author: sysadmin
 *
 * Created on October 5, 2017, 7:09 PM
 */

#ifndef MAC_H
#define MAC_H

#include <cstring>

#include "common.h"

class MacAddress {
public:
  
  MacAddress();  
  MacAddress( const mac_t& mac );
  MacAddress( const MacAddress& rhs );
  
  static bool IsBroadcast( const mac_t& mac );
  static bool IsBroadcast( const MacAddress& mac );
  
  static bool IsMulticast( const mac_t& mac );
  static bool IsMulticast( const MacAddress& mac );
  
  const mac_t& Value() const;
  
  bool operator==( const mac_t& rhs ) const;
  bool operator==( const MacAddress& rhs ) const;
  
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

#endif /* MAC_H */

