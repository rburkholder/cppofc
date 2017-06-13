/* 
 * File:   ofp_features_request.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 * 
 * Created on June 6, 2017, 10:21 AM
 */

#include <iostream>

#include "ofp_header.h"
#include "ofp_switch_features.h"
#include "datapathid.h"

namespace codec {

ofp_switch_features::ofp_switch_features( const ofp141::ofp_switch_features& packet )
: m_packet( packet )
{
  datapathid id( packet.datapath_id );
  std::cout << "ofp_features: " 
    << id << "," 
    << packet.n_buffers << ","
    << (uint16_t)packet.n_tables << ","
    << "FS=" << ((packet.capabilities & ofp141::ofp_capabilities::OFPC_FLOW_STATS) ? "1" : "0") << ","
    << "TS=" << ((packet.capabilities & ofp141::ofp_capabilities::OFPC_TABLE_STATS) ? "1" : "0") << ","
    << "PS=" << ((packet.capabilities & ofp141::ofp_capabilities::OFPC_PORT_STATS) ? "1" : "0") << ","
    << "GS=" << ((packet.capabilities & ofp141::ofp_capabilities::OFPC_GROUP_STATS) ? "1" : "0") << ","
    << "IP=" << ((packet.capabilities & ofp141::ofp_capabilities::OFPC_IP_REASM) ? "1" : "0") << ","
    << "QS=" << ((packet.capabilities & ofp141::ofp_capabilities::OFPC_QUEUE_STATS) ? "1" : "0") << ","
    << "PB=" << ((packet.capabilities & ofp141::ofp_capabilities::OFPC_PORT_BLOCKED) ? "1" : "0") << ","
    << "BU=" << ((packet.capabilities & ofp141::ofp_capabilities::OFPC_BUNDLES) ? "1" : "0") << ","
    << "FM=" << ((packet.capabilities & ofp141::ofp_capabilities::OFPC_FLOW_MONITORING) ? "1" : "0")
    << std::endl;
}

ofp_switch_features::~ofp_switch_features( ) {
}

vByte_t ofp_switch_features::CreateRequest( vByte_t v ) {
  v.resize( sizeof( ofp_header::ofp_header_ ) );
  auto* p = new( v.data() ) ofp_header::ofp_header_;
  p->init();
  p->type = ofp141::ofp_type::OFPT_FEATURES_REQUEST;
  ofp_header::NewXid( *p );
  return v;
}

} // namespace codec