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

#include <iostream>

#include <boost/endian/arithmetic.hpp>

#include "../hexdump.h"

namespace protocol {
namespace arp {

// http://www.tcpipguide.com/free/t_ARPMessageFormat.htm
enum OpCode { ArpRequest=1, ArpReply, RARPRequest, RARPReply, 
DRARPRequest, DRARPReply, DRARPError, InARPRequest, InARPReply };

// ** Header

struct Header_ { // used to overlay inbound data
  boost::endian::big_uint16_t m_typeHardware;
  boost::endian::big_uint16_t m_typeProtocol;
  boost::endian::big_uint8_t m_lenHardware;
  boost::endian::big_uint8_t m_lenProtocol;
  boost::endian::big_uint16_t m_opcode;
  uint8_t m_data[0]; // placeholder for variable length content
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
  
private:
  uint8_t m_lenHardware;
  uint8_t m_lenProtocol;
  uint8_t* m_addrHardwareSender;
  uint8_t* m_addrProtocolSender;
  uint8_t* m_addrHardwareTarget;
  uint8_t* m_addrProtocolTarget;
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

} // namespace arp
} // namespace protocol

#endif /* ARP_H */

