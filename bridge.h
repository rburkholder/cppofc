/* 
 * File:   bridge.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 * 
 * Created on September 22, 2017, 6:05 PM
 */

#ifndef BRIDGE_H
#define BRIDGE_H

#include <cstdint>
#include <unordered_map>

class bridge {
public:
  bridge( );
  virtual ~bridge( );
private:
  
  typedef uint8_t mac_t[ 6 ];
  struct port {
    
  };
  
  //typedef std::unordered_map<mac_t>

};

#endif /* BRIDGE_H */

