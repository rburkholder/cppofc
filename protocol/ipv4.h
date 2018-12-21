/*
 * File:   ipv4.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on November 7, 2018, 7:57 PM
 */

#ifndef IPV4_H
#define IPV4_H

#include <ostream>

#include <boost/endian/arithmetic.hpp>

#include "../hexdump.h"

#include "ipv4/address.h"

// https://en.wikipedia.org/wiki/IPv4

namespace protocol {
namespace ipv4 {

namespace endian=boost::endian;

// ** Header_

struct header_ { // used to overlay inbound data
  uint8_t version_ihl;
  uint8_t qos;
  endian::big_uint16_t length;
  endian::big_uint16_t identification;
  endian::big_uint16_t flags_fragment;
  endian::big_uint8_t ttl;
  endian::big_uint8_t protocol;
  endian::big_uint16_t checksum;
  address_t source_ip;
  address_t destination_ip;
  uint8_t options[0];

  uint8_t ihl() const { return version_ihl & 0x0f; }
  uint8_t version() const { return version_ihl >> 4; }
  uint8_t dscp() const { return qos >> 2; }
  uint8_t ecn() const { return qos & 0x03; }
  uint16_t offset() const {
    uint16_t flags_fragment_ = flags_fragment;
    return flags_fragment_ & 0x1fff;
  }
  bool df() const {
    uint16_t flags_fragment_ = flags_fragment;
    return ( 0 < ( flags_fragment_ & 0x400 ) );
  }
  bool mf() const {
    uint16_t flags_fragment_ = flags_fragment;
    return ( 0 < ( flags_fragment_ & 0x200 ) );
  }

  uint8_t& data() {
    assert( 5 <= ihl() );
    return 5 == ihl() ? options[0] : ( options + ( ( ihl() - 5 ) * 4 ) )[0];
  }

  uint16_t data_len() const { return (uint16_t)length - ( ihl() * 4 ); }

  bool validate() const;
};

// ** Header

class Header {
  friend std::ostream& operator<<( std::ostream&, const Header& );
public:

  Header( const header_& );
  virtual ~Header();

  bool Validate( uint16_t len ) const;

protected:
private:
  const header_& m_header;

  std::ostream& Emit( std::ostream& stream ) const;
};

std::ostream& operator<<( std::ostream& stream, const Header& header );

// ** Packet

class Packet {
  friend std::ostream& operator<<( std::ostream&, const Packet& );
public:

  Packet( uint8_t&, uint16_t len );  // need a way to determine whether to initialize or not
  virtual ~Packet();

  const header_& GetHeader() {
    return *m_pheader;
  }
  uint8_t& GetData() {
    return m_pheader->data();
  }

protected:
private:

  header_* m_pheader;
  //Content m_Content;

};

} // namespace ipv4
} // namespace protocol

#endif /* IPV4_H */
