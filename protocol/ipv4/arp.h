/*
 * File:   arp.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on 2017/09/20, 4:53 PM
 */

#ifndef ARP_H
#define ARP_H

/*
 * protocol processing:
 * http://www.danzig.jct.ac.il/tcp-ip-lab/ibm-tutorial/3376f41.gif
 */

//#include <string>
#include <ostream>
#include<unordered_map>

#include <boost/endian/arithmetic.hpp>

#include "../../hexdump.h"
#include "../ethernet/address.h"
#include "address.h"


namespace protocol {
namespace arp {

// https://tools.ietf.org/html/rfc826
// http://www.tcpipguide.com/free/t_ARPMessageFormat.htm
enum OpCode { ArpRequest=1, ArpReply, RARPRequest, RARPReply,
DRARPRequest, DRARPReply, DRARPError, InARPRequest, InARPReply };

typedef protocol::ethernet::mac_t mac_t;
typedef protocol::ethernet::MacAddress MacAddress;

// ** Header

struct Header_ { // used to overlay inbound data
  boost::endian::big_uint16_t m_typeHardware; // typically 1 for ethernet
  boost::endian::big_uint16_t m_typeProtocol; // typically 0x0800 for ethertype
  boost::endian::big_uint8_t m_lenHardware;   // typically 6 for mac
  boost::endian::big_uint8_t m_lenProtocol;   // typically 4 for ipv4
  boost::endian::big_uint16_t m_opcode;       // 1 for arp request, 2 for arp reply
  uint8_t m_data[0]; // placeholder for variable length content
  bool IsEtherIpv4() {
    return
              ( 1 == m_typeHardware )
      && ( 0x0800 == m_typeProtocol )
      &&      ( 6 == m_lenHardware )
      &&      ( 4 == m_lenProtocol );
  }
};

class Header {
  friend std::ostream& operator<<( std::ostream&, const Header& );
public:

  Header( const Header_& );
  virtual ~Header();

private:
  const Header_& m_header;

  std::ostream& Emit( std::ostream& stream ) const;
};

std::ostream& operator<<( std::ostream& stream, const Header& header );

// ** Content

class Content {
  friend std::ostream& operator<<( std::ostream&, const Content& );
public:

  Content();
  virtual ~Content();

  void Init( Header_& );

  std::ostream& Emit( std::ostream& stream ) const;

protected:
  uint8_t m_lenHardware;
  uint8_t m_lenProtocol;
  uint8_t* m_addrHardwareSender;
  uint8_t* m_addrProtocolSender;
  uint8_t* m_addrHardwareTarget;
  uint8_t* m_addrProtocolTarget;
private:
};

std::ostream& operator<<( std::ostream& stream, const Content& content );

// ** Packet

class Packet {
  friend std::ostream& operator<<( std::ostream&, const Packet& );
public:

  Packet( uint8_t& );  // need a way to determine whether to initialize or not
  virtual ~Packet();

protected:
private:

  Header_* m_pHeader_;
  Content m_Content;

  std::ostream& Emit( std::ostream& stream ) const {
    stream << "arp: " << *m_pHeader_ << m_Content;
    return stream;
  }

};


// ** IPv4

typedef protocol::ipv4::address_t ipv4_t;

struct IPv4_Ether_ {
  Header_ header;
  mac_t macSender;
  ipv4_t ipv4Sender;
  mac_t macTarget;
  ipv4_t ipv4Target;
};

class IPv4_Ether {
public:
  IPv4_Ether( uint8_t& );  // need a way to determine whether to initialize or not
  virtual ~IPv4_Ether();

  const mac_t& MacSender() { return m_ipv4->macSender; }
  const mac_t& MacTarget() { return m_ipv4->macTarget; }
  const ipv4_t& IPv4Sender() { return m_ipv4->ipv4Sender; }
  const ipv4_t& IPv4Target() { return m_ipv4->ipv4Target; }

protected:
private:
  IPv4_Ether_* m_ipv4;
};

// ** Cache

class Cache {
  friend std::ostream& operator<<( std::ostream&, const Packet& );
public:
  void Update( IPv4_Ether& );
protected:
private:
  typedef std::unordered_map<protocol::ipv4::address,MacAddress> mapArp_t;

  mapArp_t m_mapArp;
};

} // namespace arp
} // namespace protocol

#endif /* ARP_H */

