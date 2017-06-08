/* 
 * File:   ofp_async_config.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 * 
 * Created on June 8, 2017, 9:20 AM
 */

#include <array>
#include <string>
#include <iostream>

#include "ofp_async_config.h"

namespace codec {

ofp_async_config::ofp_async_config( const ofp141::ofp_async_config& packet ) 
: m_packet( packet )
{
  Properties( [this]( const ofp141::ofp_async_config_prop_header& property ) { Decode( property ); } );
}

ofp_async_config::~ofp_async_config( ) {
}

void ofp_async_config::Properties( funcPropertyHeader_t f ) const {
  auto   p = (uint8_t*)m_packet.properties;
  auto end = (uint8_t*)m_packet.properties + m_packet.header.length - sizeof( ofp141::ofp_header );
  while ( p < end ) {
    auto p2 = (ofp141::ofp_async_config_prop_header*) p;
    f( *p2 );
    p += p2->length;
  }
}

namespace {
  std::array<std::string, 12> rNames = {
    "Packet In Slave",
    "Packet In Master",
    "Port Status Slave",
    "Port Status Master",
    "Flow Removed Slave",
    "Flow Removed Master",
    "Role Status Slave",
    "Role Status Master",
    "Table Status Slave",
    "Table Status Master",
    "RequestForward Slave",
    "RequestForward Master"
  };
}

void ofp_async_config::Decode( const ofp141::ofp_async_config_prop_header& property ) {
  switch ( property.type ) {
    case ofp141::ofp_async_config_prop_type::OFPACPT_PACKET_IN_SLAVE:  // ofp_packet_in_reason
    case ofp141::ofp_async_config_prop_type::OFPACPT_PACKET_IN_MASTER:
    case ofp141::ofp_async_config_prop_type::OFPACPT_PORT_STATUS_SLAVE:  // ofp_port_reason
    case ofp141::ofp_async_config_prop_type::OFPACPT_PORT_STATUS_MASTER:
    case ofp141::ofp_async_config_prop_type::OFPACPT_FLOW_REMOVED_SLAVE: //ofp_flow_removed_reason
    case ofp141::ofp_async_config_prop_type::OFPACPT_FLOW_REMOVED_MASTER:
    case ofp141::ofp_async_config_prop_type::OFPACPT_ROLE_STATUS_SLAVE:  // ofp_controller_role_reason
    case ofp141::ofp_async_config_prop_type::OFPACPT_ROLE_STATUS_MASTER:
    case ofp141::ofp_async_config_prop_type::OFPACPT_TABLE_STATUS_SLAVE:  // ofp_table_reason
    case ofp141::ofp_async_config_prop_type::OFPACPT_TABLE_STATUS_MASTER:
    case ofp141::ofp_async_config_prop_type::OFPACPT_REQUESTFORWARD_SLAVE: // ofp_requestforward_reason
    case ofp141::ofp_async_config_prop_type::OFPACPT_REQUESTFORWARD_MASTER:
    {
      auto& reason( (ofp141::ofp_async_config_prop_reasons&) property );
      std::cout << rNames[property.type] << "=" << reason.mask << std::endl;
    }
      break;
    case ofp141::ofp_async_config_prop_type::OFPTFPT_EXPERIMENTER_SLAVE:
    {
      auto& ex( (ofp141::ofp_async_config_prop_experimenter&) property);
      std::cout << "Experimenter Slave=" << ex.experimenter << "," << ex.exp_type << std::endl;
    }
      break;    
    case ofp141::ofp_async_config_prop_type::OFPTFPT_EXPERIMENTER_MASTER:
    {
      auto& ex( (ofp141::ofp_async_config_prop_experimenter&) property);
      std::cout << "Experimenter Master=" << ex.experimenter << "," << ex.exp_type << std::endl;
    }
      break;    
  }
}

} // namespace codec