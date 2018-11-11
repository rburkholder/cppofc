/* 
 * File:   ipv6.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on November 11, 2018, 12:53 PM
 */

#ifndef IPV6_H
#define IPV6_H

#include <ostream>

#include <boost/endian/arithmetic.hpp>

#include "../hexdump.h"

// https://en.wikipedia.org/wiki/IPv6_packet

namespace protocol {
namespace ipv6 {

namespace endian=boost::endian;

// ** Header_

struct Header_ { // used to overlay inbound data
  endian::big_uint32_t version_class_flow;
  endian::big_uint16_t payload_length;
  uint8_t next_header;
  uint8_t hop_limit;
  uint8_t ip_src[16];
  uint8_t ip_dst[16];
  uint8_t headers[0];
  
  uint16_t version() const { uint32_t tmp = version_class_flow; return ( tmp & 0xf0000000 ) >> 28; }
  uint16_t traffic_class() const { uint32_t tmp = version_class_flow; return ( tmp & 0x0ff00000 ) > 20; }
  uint32_t flow_label() const { uint32_t tmp = version_class_flow; return tmp & 0x000fffff; }
  
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
  
  const Header_& GetHeader() {
    return *m_pHeader_;
  }
  //uint8_t& GetData() {
  //  return m_pHeader_->headers();
  //}

protected:
private:
  
  Header_* m_pHeader_;
  //Content m_Content;
  
//  std::ostream& Emit( std::ostream& stream ) const {
//    stream << "ipv6b: " << *m_pHeader_;
//    return stream;
//  }

};

} // namespace ipv6
} // namespace protocol

#endif /* IPV6_H */

