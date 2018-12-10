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

#include <boost/endian/arithmetic.hpp>

//#include "../common.h"
#include "../protocol/ethernet/address.h"
#include "../hexdump.h"

namespace protocol {
namespace ethernet {

// https://en.wikipedia.org/wiki/Ethernet_frame
// https://en.wikipedia.org/wiki/EtherType
// https://en.wikipedia.org/wiki/IEEE_802.1Q

enum Ethertype {
    ipv4=0x0800
  , arp=0x0806
  , ieee8021q=0x8100 // vlan
  , pause=0x8808 // ethernet flow control
  , lacp=0x8809
  , mpls_unicast=0x8847
  , mpls_multicast=0x8848
  , ieee8021ad=0x88ab // QinQ
  , lldp=0x88cc
  , ipv6=0x86dd
  , ieee8021ah=0x88e7 // provider backbone bridges
  , ieee1588=0x88f7 // precision time protocol
  , ieee8021ag=0x8902 // Connectivity Fault Management
  , ectp=0x9000 // Ethernet Configuration Testing Protocol
  , vlan_double_tagged=0x9100
};

typedef protocol::ethernet::mac_t mac_t;

struct header_ {
  //uint8_t m_padding[ 2 ]; // supplied by ofp_packet_in
  mac_t m_macDest;
  mac_t m_macSrc;
  boost::endian::big_uint16_t m_ethertype;
  uint8_t m_message[0];
};

class header {
  friend std::ostream& operator<<( std::ostream&, const header& );
public:

  header( uint8_t& ); // packet data, determine whether to init or not
  virtual ~header( );

  uint16_t GetEthertype() const {
    return m_pHeader->m_ethertype;
  }

  const mac_t& GetDstMac() const { return m_pHeader->m_macDest; }
  const mac_t& GetSrcMac() const { return m_pHeader->m_macSrc; }

  uint8_t& GetMessage() {
    return (m_pHeader->m_message)[0];
  }

  std::ostream& Emit( std::ostream& stream ) const;

private:

  header_* m_pHeader;

};

} // namespace ethernet
} // namespace protocol

#endif /* ETHERNET_H */

