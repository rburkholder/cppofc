/* 
 * File:   ofp_packet_out.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on June 13, 2017, 4:18 PM
 */

#ifndef OFP_PACKET_OUT_H
#define OFP_PACKET_OUT_H

#include "../openflow/openflow-spec1.4.1.h"

namespace codec {

class ofp_packet_out { // page 129 v1.4.1 s7.3.6
public:
  
  struct ofp_packet_out_: public ofp141::ofp_packet_out {
    ofp_packet_out_( bool bInit = false ) {
    }
  };
  
  struct ofp_action_header_: public ofp141::ofp_action_header {
    ofp_action_header_( bool bInit = false ) {
      
    }
  };
  
  struct ofp_action_output_: public ofp141::ofp_action_output { // includes ofp_action_header
    ofp_action_output_( bool bInit = false ) {
      if ( bInit ) {
        type = ofp141::ofp_action_type::OFPAT_OUTPUT;
        len = sizeof( ofp141::ofp_action_output );
        port = 0;  // output port needs to be changed
        max_len = 0;  // needs to be updated when port is OFPP_CONTROLLER
      }
    }
  };
  
  ofp_packet_out( );
  virtual ~ofp_packet_out( );
private:

};

} // namespace codec

#endif /* OFP_PACKET_OUT_H */

