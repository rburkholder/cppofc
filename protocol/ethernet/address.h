/*
 * File:   mac.h
 * Author: sysadmin
 *
 * Created on October 5, 2017, 7:09 PM
 */

#ifndef MAC_ADDRESS_H
#define MAC_ADDRESS_H

#include <cstring>
#include <cstdint>
#include <vector>
#include <string>

namespace protocol {
namespace ethernet {

typedef uint8_t mac_t[ 6 ];

void ConvertStringToMac( const std::string& src, mac_t& dst ); // hh:hh:hh:hh:hh:hh

class MacAddress {
public:

  MacAddress();
  MacAddress( const mac_t& mac );
  MacAddress( const MacAddress& rhs );
  MacAddress( const std::string& );

  bool IsBroadcast() const;
  static bool IsBroadcast( const mac_t& mac );
  static bool IsBroadcast( const MacAddress& mac );

  bool IsMulticast() const;
  static bool IsMulticast( const mac_t& mac );
  static bool IsMulticast( const MacAddress& mac );

  bool IsAllZero() const; // ARP Probe, RFC 5227
  static bool IsAllZero( const mac_t& mac );
  static bool IsAllZero( const MacAddress& mac );

  const mac_t& Value() const;

  bool operator==( const mac_t& rhs ) const;
  bool operator==( const MacAddress& rhs ) const;

protected:
private:
  mac_t m_mac;
};

} // namespace ethernet
} // namespace protocol

// http://en.cppreference.com/w/cpp/utility/hash
namespace std {
  template<> struct hash<protocol::ethernet::mac_t> {
    typedef protocol::ethernet::mac_t argument_type;
    typedef std::size_t result_type;
    result_type operator()( const argument_type& mac ) const {
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

// http://en.cppreference.com/w/cpp/utility/hash
namespace std {
  template<> struct hash<protocol::ethernet::MacAddress> {
    typedef protocol::ethernet::MacAddress argument_type;
    typedef std::size_t result_type;
    result_type operator()( const argument_type& mac ) const {
      return std::hash<protocol::ethernet::mac_t>{}( mac.Value() );
      }
    };
}

#endif /* MAC_ADDRESS_H */

