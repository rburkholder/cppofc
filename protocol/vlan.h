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
  boost::endian::big_uint16_t m_tpid;
  boost::endian::big_uint16_t m_tci;
};

class vlan {
  friend std::ostream& operator<<( std::ostream&, const vlan& );
public:

  vlan( uint8_t& );
  virtual ~vlan();
  
  uint8_t  GetPCP() const { return ( m_pVlan->m_tci & 0xE000 ) >> 13; } // aka class of service
  uint8_t  GetDEI() const { return ( m_pVlan->m_tci & 0x1000 ) >> 12; } // drop eligible indicator
  uint16_t GetVID() const { return ( m_pVlan->m_tci & 0x0FFF ); } // vlan identifier
  
  std::ostream& Emit( std::ostream& stream ) const;
  
private:
  vlan_* m_pVlan;
};
  
} // namespace ethernet

#endif /* VLAN_H */

