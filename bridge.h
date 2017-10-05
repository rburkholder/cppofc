/* 
 * File:   bridge.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 * 
 * Created on September 22, 2017, 6:05 PM
 */

#ifndef BRIDGE_H
#define BRIDGE_H

#include <unordered_map>

#include "common.h"
#include "mac.h"

class Bridge {
public:
  
  enum MacStatus { StatusQuo, Broadcast, Learned, Moved }; // add 'Flap' ?
  
  Bridge( );
  virtual ~Bridge( );
  
  MacStatus Update( nPort_t nPort, const mac_t& macSource );
  nPort_t Lookup( const mac_t& mac );
  
private:
  
  struct MacInfo {
    nPort_t m_inPort;
    MacInfo( nPort_t inPort ): m_inPort( inPort ) {}
  };
  
  // change to pimpl for use with MacAddress
  typedef std::unordered_map<MacAddress,MacInfo> mapMac_t;
  
  mapMac_t m_mapMac;

};

#endif /* BRIDGE_H */

