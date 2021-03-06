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

// https://en.wikipedia.org/wiki/Transmission_Control_Protocol

namespace protocol {
namespace tcp {

namespace endian=boost::endian;

// ** Header_

struct header_ { // used to overlay inbound data
  endian::big_uint16_t src_port;
  endian::big_uint16_t dst_port;
  endian::big_uint32_t sequence_num;
  endian::big_uint32_t acknowledgement_num;
  endian::big_uint16_t aggregate;
  endian::big_uint16_t window;
  endian::big_uint16_t checksum;
  endian::big_uint16_t urgent;
  uint8_t options[0]; // http://www.iana.org/assignments/tcp-parameters/tcp-parameters.xhtml

  uint16_t offset() const {
    uint16_t aggregate_( aggregate );
    uint8_t offset_( aggregate_ >> 12 );
    assert(  5 <= offset_ );
    assert( 15 >= offset_ );
    return ((uint16_t)aggregate) >> 12;
  }

  uint16_t flags() const {
    return ((uint16_t)aggregate) & 0x1ff;
  }

  bool  ns() const { return 0 < (((uint16_t)aggregate) & 0x100); } // ECN-nonce - concealment protection
  bool cwr() const { return 0 < (((uint16_t)aggregate) & 0x080); } // Congestion Window Reduced (CWR)
  bool ece() const { return 0 < (((uint16_t)aggregate) & 0x040); } // ECN-Echo
  bool urg() const { return 0 < (((uint16_t)aggregate) & 0x020); } // Urgent pointer field is significant
  bool ack() const { return 0 < (((uint16_t)aggregate) & 0x010); } // Acknowledgment field is significant
  bool psh() const { return 0 < (((uint16_t)aggregate) & 0x008); } // push the buffered data to the receiving application
  bool rst() const { return 0 < (((uint16_t)aggregate) & 0x004); } // Reset the connection
  bool syn() const { return 0 < (((uint16_t)aggregate) & 0x002); } // Synchronize sequence numbers
  bool fin() const { return 0 < (((uint16_t)aggregate) & 0x001); } // Last packet from sender.

  uint8_t& data() {
    uint16_t offset_( offset() );
    return 5 == offset_ ? options[0] : ( options + ( ( offset_ - 5 ) * 4 ) )[0];
  }
};

// ** Header

class Header {
  friend std::ostream& operator<<( std::ostream&, const Header& );
public:

  Header( const header_& );
  virtual ~Header();

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

  Packet( uint8_t&, uint16_t len );
  virtual ~Packet();

  const header_& GetHeader() {
    return *m_pheader_;
  }
  uint8_t& GetData() {
    return m_pheader_->data();
  }

protected:
private:

  header_* m_pheader_;
  //Content m_Content;

};

} // namespace tcp
} // namespace protocol

#endif /* TCP_H */

