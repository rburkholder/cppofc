/* 
 * File:   vlan.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 * 
 * Created on September 22, 2017, 4:44 PM
 */

#include <iostream>

#include "vlan.h"

namespace ethernet {

vlan::vlan( uint8_t& info ) {
  m_pVlan = new( &info ) vlan_;
}

vlan::~vlan( ) {
}

std::ostream& vlan::Emit( std::ostream& stream ) const {
  stream
    //<< std::hex
    //<< "tpid=0x" << GetTPid()
    //<< std::dec
    << "pcp=" << GetPCP()
    << ",dei=" << GetDEI()
    << ",vid=" << GetVID()
    << ".ethertype=0x" << std::hex << GetEthertype() << std::dec
  ;
  return stream;
}

std::ostream& operator<<( std::ostream& stream, const vlan& vlan ) {
  stream << "vlan: ";
  vlan.Emit( stream );
  return stream;
}

} // namespace ethernet