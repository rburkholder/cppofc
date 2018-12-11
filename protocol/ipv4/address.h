/*
 * File:   address.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on December 9, 2018, 4:54 PM
 */

#ifndef IPV4_ADDRESS_H
#define IPV4_ADDRESS_H

#include <string>
#include <ostream>

namespace protocol {
namespace ipv4 {

typedef uint8_t address_t[4];

std::ostream& operator<<( std::ostream& stream, const address_t& ); // not getting called with native address_t

void ConvertStringToIPv4( const std::string& sIPv4, address_t& ipv4 );

class address {
  friend std::ostream& operator<<( std::ostream&, const address& );
public:
  address();
  address( const address_t& );
  address( const address& );
  virtual ~address( );

  const address_t& value() const { return m_ipv4; }

  bool operator==( const address_t& ) const;
  bool operator==( const address& ) const;

protected:
private:
  address_t m_ipv4;

  std::ostream& Emit( std::ostream& stream ) const;

};

} // namespace ipv4
} // namespace protocol

// http://en.cppreference.com/w/cpp/utility/hash
namespace std {
  template<> struct hash<protocol::ipv4::address_t> {
    typedef protocol::ipv4::address_t argument_type;
    typedef std::size_t result_type;
    result_type operator()( const argument_type& ipv4 ) const {
      result_type value;
      value  = ipv4[0]; value <<= 8;
      value ^= ipv4[1]; value <<= 8;
      value ^= ipv4[2]; value <<= 8;
      value ^= ipv4[3];
      return value;
      }
    };
}

// http://en.cppreference.com/w/cpp/utility/hash
namespace std {
  template<> struct hash<protocol::ipv4::address> {
    typedef protocol::ipv4::address argument_type;
    typedef std::size_t result_type;
    result_type operator()( const argument_type& address ) const {
      return std::hash<protocol::ipv4::address_t>{}( address.value() );
      }
    };
}

#endif /* IPV4_ADDRESS_H */

