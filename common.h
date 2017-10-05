
/* 
 * File:   common.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on June 5, 2017, 5:03 PM
 */

#ifndef COMMON_H
#define COMMON_H

#include <cstdint>
#include <vector>

typedef uint32_t nPort_t;  // switch port number
typedef uint8_t mac_t[ 6 ];


// http://en.cppreference.com/w/cpp/utility/hash
namespace std {
  template<> struct hash<const mac_t&> {
    typedef const mac_t&  argument_type;
    typedef std::size_t result_type;
    result_type operator()( argument_type mac ) const {
      result_type value;
      value  = mac[0]; value <<= 4;
      value ^= mac[5]; value <<= 5;
      value ^= mac[1]; value <<= 5;
      value ^= mac[4]; value <<= 5;
      value ^= mac[2]; value <<= 5;
      value ^= mac[3];
      return value;
      }
    };
}


typedef std::vector<uint8_t> vByte_t;  // rename to octet?

#endif /* COMMON_H */

