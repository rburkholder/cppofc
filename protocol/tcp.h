/* 
 * File:   tcp.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on November 8, 2018, 3:30 PM
 */

#ifndef TCP_H
#define TCP_H

#include <ostream>

#include <boost/endian/arithmetic.hpp>

#include "../hexdump.h"

// https://en.wikipedia.org/wiki/Transmission_Control_Protocol

namespace protocol {
namespace tcp {

namespace endian=boost::endian;

// ** Header_

struct Header_ { // used to overlay inbound data
  endian::big_uint16_t src_port;
  endian::big_uint16_t dst_port;
  endian::big_uint32_t sequence_num;
  endian::big_uint32_t acknowledgement_num;
  endian::big_uint16_t aggregate;
  endian::big_uint16_t window;
  endian::big_uint16_t checksum;
  endian::big_uint16_t urgent;
  uint8_t options[0];
  
  uint8_t offset() const {
    assert( 5 <= offset() );
    return ((uint16_t)aggregate) >> 12;
  }
  
  uint16_t flags() const {
    return ((uint16_t)aggregate) & 0x1ff;
  }
  
  bool  ns() const { return 0 < ((uint16_t)aggregate & 0x100); }
  bool cwr() const { return 0 < ((uint16_t)aggregate & 0x080); }
  bool ece() const { return 0 < ((uint16_t)aggregate & 0x040); }
  bool urg() const { return 0 < ((uint16_t)aggregate & 0x020); }
  bool ack() const { return 0 < ((uint16_t)aggregate & 0x010); }
  bool psh() const { return 0 < ((uint16_t)aggregate & 0x008); }
  bool rst() const { return 0 < ((uint16_t)aggregate & 0x004); }
  bool syn() const { return 0 < ((uint16_t)aggregate & 0x002); }
  bool fin() const { return 0 < ((uint16_t)aggregate & 0x001); }
  
  uint8_t& data() {
    return 5 == offset() ? options[0] : ( options + ( ( offset() - 5 ) * 4 ) )[0];
  }
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
    return m_pHeader_->data();
  }

protected:
private:
  
  Header_* m_pHeader_;
  //Content m_Content;
  
  std::ostream& Emit( std::ostream& stream ) const {
    stream << "tcp: " << *m_pHeader_;
    return stream;
  }

};

} // namespace tcp
} // namespace protocol

#endif /* TCP_H */

