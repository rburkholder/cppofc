/* 
 * File:   ofp_features_request.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 * 
 * Created on June 6, 2017, 10:21 AM
 */

#include <iostream>

#include "ofp_switch_features.h"

namespace codec {

ofp_switch_features::ofp_switch_features( const ofp141::ofp_switch_features& packet )
: m_packet( packet )
{
  std::cout << "ofp_features: " 
    << packet.datapath_id << "," 
    << packet.n_buffers << ","
    << packet.n_tables << ","
    << packet.capabilities 
    << std::endl;
}

ofp_switch_features::~ofp_switch_features( ) {
}

} // namespace codec