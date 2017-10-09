/* 
 * File:   ofp_packet_out.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net * 
 * Created on June 13, 2017, 4:18 PM
 */

#include <cstring>
#include <vector>

#include "ofp_packet_out.h"

namespace codec {

void ofp_packet_out::build( 
  vByte_t& vDestination, uint32_t nPort, size_t nOctets, void* src, ofp141::ofp_port_no portOutput
) {
  
  size_t size // need to check for packing and padding issues
    = sizeof( ofp_packet_out_ )
    + sizeof( ofp_action_output_ ) 
    + nOctets; 
  
  vDestination.resize( size );
  
  auto* pOfpPacketOut = new( vDestination.data() ) ofp_packet_out_;
  pOfpPacketOut->init( size, nPort );
  
  // will need to loop this structure for multiple actions
  auto* pActionOutput = new( pOfpPacketOut->actions ) ofp_action_output_;
  //pActionOutput->init( ofp141::ofp_port_no::OFPP_FLOOD );
  //pActionOutput->init( ofp141::ofp_port_no::OFPP_ALL );
  pActionOutput->init( portOutput );
  
  // TODO: pull the xid out of the packet_in message.
  
  auto* pPacket = new( pOfpPacketOut->actions ) uint8_t;
  pPacket += sizeof( ofp_action_output_ );
  /* dest = */ std::memcpy( pPacket, src, nOctets );
  
} 

} // namespace codec
