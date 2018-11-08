/* 
 * File:   udp.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on November 8, 2018, 1:26 PM
 */

#ifndef UDP_H
#define UDP_H

#include <ostream>

#include <boost/endian/arithmetic.hpp>

#include "../hexdump.h"

// https://en.wikipedia.org/wiki/IPv4

namespace protocol {
namespace udp {

namespace endian=boost::endian;

// ** Header_

struct Header_ { // used to overlay inbound data
  endian::big_int16_t src_port;
  endian::big_int16_t dst_port;
  endian::big_int16_t length;
  endian::big_int16_t checksum;
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
  
  const Header_& GetHeader() {
    return *m_pHeader_;
  }
  uint8_t& GetData() {
    return m_pHeader_->data[0];
  }

protected:
private:
  
  Header_* m_pHeader_;
  //Content m_Content;
  
  std::ostream& Emit( std::ostream& stream ) const {
    stream << "udp: " << *m_pHeader_;
    return stream;
  }

};

} // namespace udp
} // namespace protocol

#endif /* UDP_H */
