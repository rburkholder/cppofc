/* 
 * File:   vlan.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 * 
 * Created on September 22, 2017, 4:44 PM
 */

#ifndef VLAN_H
#define VLAN_H

#include <cstdint>

#include <boost/endian/arithmetic.hpp>

namespace ethernet {

struct vlan_ {
  //boost::endian::big_uint16_t m_tpid; // consumed in the ethernet header
  boost::endian::big_uint16_t m_tci;
  boost::endian::big_uint16_t m_ethertype;
  uint8_t m_data[0]; // placeholder for variable length content
};

class vlan {
  friend std::ostream& operator<<( std::ostream&, const vlan& );
public:

  vlan( uint8_t& );
  virtual ~vlan();
  
  //uint16_t GetTPid() const { return ( m_pVlan->m_tpid ); }
  uint16_t GetPCP() const { return ( ((uint16_t)(m_pVlan->m_tci)) & 0xE000 ) >> 13; } // aka IEEE 802.1p class of service 
  uint16_t GetDEI() const { return ( ((uint16_t)(m_pVlan->m_tci)) & 0x1000 ) >> 12; } // drop eligible indicator
  uint16_t GetVID() const { return ( ((uint16_t)(m_pVlan->m_tci)) & 0x0FFF ); } // vlan identifier
  
  uint16_t GetEthertype() const {
    return m_pVlan->m_ethertype;
  }
  
  uint8_t& GetMessage() { 
    return (m_pVlan->m_data)[0];
  }
  
  std::ostream& Emit( std::ostream& stream ) const;
  
private:
  vlan_* m_pVlan;
};
  
} // namespace ethernet

#endif /* VLAN_H */

