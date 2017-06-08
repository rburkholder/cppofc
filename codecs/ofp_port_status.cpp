/* 
 * File:   ofp_port_status.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 * 
 * Created on June 8, 2017, 11:12 AM
 */

#include <iostream>

#include "ofp_port_status.h"

namespace codec {

ofp_port_status::ofp_port_status( const ofp141::ofp_port_status& packet )
: m_packet( packet )
{
  std::cout 
    << "port "
    << packet.desc.name
    << " reason=" << (uint16_t)packet.reason 
    << std::endl;
}

ofp_port_status::~ofp_port_status( ) {
}

}