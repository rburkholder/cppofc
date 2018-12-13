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

// https://en.wikipedia.org/wiki/Dynamic_Host_Configuration_Protocol

namespace protocol {
namespace ipv4 {
namespace dhcp {

namespace endian=boost::endian;

// ** Header_

struct Header_ { // used to overlay inbound data, https://www.ietf.org/rfc/rfc2131.txt
  uint8_t op; // 1 = BOOTREQUEST, 2 = BOOTREPLY (BoOTP messages)
  uint8_t htype; // 1 = 10mb ethernet., hardware address
  uint8_t hlen;  // 6 or 10mb ethernet), Hardware address length
  uint8_t hops;  // Client sets to zero, may be used by relay agents
  endian::big_int32_t xid; // random transaction id, set by client
  endian::big_int16_t secs; // client, seconds elapsed since client began address acquisition or renewal process.
  endian::big_int16_t flags; // flags -  0x80 - broadcast
  endian::big_int32_t ciaddr; // Client IP address; only filled in if client is in BOUND, RENEW or REBINDING state and can respond to ARP requests.
  endian::big_int32_t yiaddr; // 'your' (client) IP address (supplied by server)
  endian::big_int32_t siaddr; // IP address of next server to use in bootstrap;
  endian::big_int32_t giaddr; // Relay agent IP address, used in booting via a relay agent.
  uint8_t chaddr[16]; // Client hardware address.
  char szname[64]; // Optional server host name, null terminated string
  char szfile[128]; // Boot file name, null terminated string; "generic" name or null in DHCPDISCOVER, fully qualified directory-path name in DHCPOFFER.
  uint8_t magic_cookie[4]; // should be 99, 130, 83 and 99
  uint8_t options[0];  // RFC 2132 gives the complete set for use in DHCP

};

// ** Header

class Header {
  friend std::ostream& operator<<( std::ostream&, const Header& );
public:

  Header( Header_& );
  virtual ~Header();

protected:
private:
  Header_& m_header;

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

};

} // namespace dhcp
} // namespace ipv4
} // namespace protocol

#endif /* DHCP_H */

