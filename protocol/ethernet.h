/* 
 * File:   ethernet.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on June 12, 2017, 4:53 PM
 */

#ifndef ETHERNET_H
#define ETHERNET_H

#include <cstdint>
//#include <cassert>
//#include <sstream>
#include <iomanip>

#include <boost/endian/arithmetic.hpp>

#include "../common.h"
#include "../hexdump.h"

namespace protocol {
  
// https://en.wikipedia.org/wiki/Ethernet_frame
// https://en.wikipedia.org/wiki/IEEE_802.1Q
  
class Ethernet {
  friend std::ostream& operator<<( std::ostream&, const Ethernet& );
public:
  
  enum Ethertype { ipv4=0x0800, arp=0x0806, ieee8021q=0x8100, ipv6=0x86dd, ieee8021ad=0x88ab };
  
  Ethernet( uint8_t& ); // packet data, determine whether to init or not
  virtual ~Ethernet( );
  
  uint16_t GetEthertype() const {
    auto p = new( &m_rOctets ) prelude;
    return p->m_ethertype;
  }
  
  private:

  struct prelude {
    uint8_t m_padding[ 2 ]; // supplied by ofp_packet_in
    uint8_t m_macDest[ 6 ];
    uint8_t m_macSrc[ 6 ];
    boost::endian::big_uint16_t m_ethertype;
  };
  
  struct vlan {
    boost::endian::big_uint16_t m_tpid;
    boost::endian::big_uint16_t m_tci;
    uint8_t GetPCP()  { return ( m_tci & 0xE000 ) >> 13; } // aka class of service
    uint8_t GetDEI()  { return ( m_tci & 0x1000 ) >> 12; } // drop eligible indicator
    uint16_t GetVID() { return ( m_tci & 0x0FFF ); } // vlan identifier
  };
  
  uint8_t& m_rOctets;
  
std::ostream& Emit( std::ostream& stream ) const {
    
    auto p = new( &m_rOctets ) prelude;
    
    // http://www.cplusplus.com/forum/windows/51591/
    std::ios_base::fmtflags oldFlags = stream.flags();
    std::streamsize         oldPrec  = stream.precision();
         char               oldFill  = stream.fill();
         
    stream << std::showbase // show the 0x prefix
           << std::internal // fill between the prefix and the number
           << std::setfill('0'); // fill with 0s

    stream 
      << "ethernet: "
      << "dstmac=" << HexDump<uint8_t*>( p->m_macDest, p->m_macDest + 6 )
      << " srcmac=" << HexDump<uint8_t*>( p->m_macSrc, p->m_macSrc + 6 )
      << " ethertype=" << std::hex << p->m_ethertype << std::dec
      << std::endl;
    
    stream.flags(oldFlags);
    stream.precision(oldPrec);
    stream.fill(oldFill);
    
    return stream;
    
  }

};

} // namespace protocol

#endif /* ETHERNET_H */

