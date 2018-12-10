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

typedef uint8_t address_t[ 6 ];

std::ostream& operator<<( std::ostream& stream, const protocol::ethernet::address_t& ); // not getting called with native address_t

void ConvertStringToMac( const std::string& src, address_t& dst ); // hh:hh:hh:hh:hh:hh

class address {
  friend std::ostream& operator<<( std::ostream&, const address& );
public:

  address();
  address( const address_t& mac );
  address( const address& rhs );
  address( const std::string& );

  bool IsBroadcast() const;
  static bool IsBroadcast( const address_t& mac );
  static bool IsBroadcast( const address& mac );

  bool IsMulticast() const;
  static bool IsMulticast( const address_t& mac );
  static bool IsMulticast( const address& mac );

  bool IsAllZero() const; // ARP Probe, RFC 5227
  static bool IsAllZero( const address_t& mac );
  static bool IsAllZero( const address& mac );

  const address_t& Value() const;

  const address& operator=( const address& rhs );
  const address& operator=( const address_t& rhs );

  bool operator==( const address_t& rhs ) const;
  bool operator==( const address& rhs ) const;

protected:
private:
  address_t m_mac;

  std::ostream& Emit( std::ostream& stream ) const;

};

} // namespace ethernet
} // namespace protocol

// http://en.cppreference.com/w/cpp/utility/hash
namespace std {
  template<> struct hash<protocol::ethernet::address_t> {
    typedef protocol::ethernet::address_t argument_type;
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
  template<> struct hash<protocol::ethernet::address> {
    typedef protocol::ethernet::address argument_type;
    typedef std::size_t result_type;
    result_type operator()( const argument_type& mac ) const {
      return std::hash<protocol::ethernet::address_t>{}( mac.Value() );
      }
    };
}

#endif /* MAC_ADDRESS_H */

