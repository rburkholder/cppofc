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

// https://en.wikipedia.org/wiki/IPv4

namespace protocol {
namespace ipv4 {

namespace endian=boost::endian;

// ** Header_

struct Header_ { // used to overlay inbound data
  uint8_t version_ihl;
  uint8_t qos;
  endian::big_int16_t length;
  endian::big_int16_t identification;
  endian::big_int16_t flags_fragment;
  uint8_t ttl;
  uint8_t protocol;
  endian::big_int16_t checksum;
  endian::big_int32_t source_ip;
  endian::big_int32_t destination_ip;
  uint8_t data[0];
};

// ** Header

class Header {
  friend std::ostream& operator<<( std::ostream&, const Header& );
public:
  
  Header( const Header_& );
  virtual ~Header();

protected:  
private:  
  const Header_& m_header;
  
  std::ostream& Emit( std::ostream& stream ) const;
};

std::ostream& operator<<( std::ostream& stream, const Header& header );

// ** Packet

class Packet {
  friend std::ostream& operator<<( std::ostream&, const Packet& );
public:
  
  Packet( uint8_t& );  // need a way to determine whether to initialize or not
  virtual ~Packet();
  
protected:
private:
  
  Header_* m_pHeader_;
  //Content m_Content;
  
  std::ostream& Emit( std::ostream& stream ) const {
    stream << "ipv4: " << *m_pHeader_;
    return stream;
  }

};

} // namespace ipv4
} // namespace protocol

#endif /* IPV4_H */

