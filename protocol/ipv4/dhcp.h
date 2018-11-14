/* 
 * File:   dhcp.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on November 13, 2018, 7:50 PM
 */

#ifndef DHCP_H
#define DHCP_H

#include <ostream>

#include <boost/endian/arithmetic.hpp>

#include "../../hexdump.h"

// https://en.wikipedia.org/wiki/Dynamic_Host_Configuration_Protocol

namespace protocol {
namespace ipv4 {
namespace dhcp {

namespace endian=boost::endian;

// ** Header_

struct Header_ { // used to overlay inbound data
  uint8_t op;
  uint8_t htype;
  uint8_t hlen;
  uint8_t hops;
  endian::big_int32_t xid;
  endian::big_int16_t secs;
  endian::big_int16_t flags;
  endian::big_int32_t ciaddr;
  endian::big_int32_t yiaddr;
  endian::big_int32_t siaddr;
  endian::big_int32_t giaddr;
  uint8_t chaddr[16];
  uint8_t legacy[192];
  endian::big_int32_t magic_cookie;
  uint8_t options[0];

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

protected:
private:
  
  Header_* m_pHeader_;
  //Content m_Content;
  
//  std::ostream& Emit( std::ostream& stream ) const {
//    stream << "ipv4: " << *m_pHeader_;
//    return stream;
//  }

};

} // namespace dhcp
} // namespace ipv4
} // namespace protocol

#endif /* DHCP_H */

