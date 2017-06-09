/* 
 * File:   ofp_flow_mod.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on June 8, 2017, 1:47 PM
 */

#ifndef OFP_FLOW_MOD_H
#define OFP_FLOW_MOD_H

#include "../openflow/openflow-spec1.4.1.h"
#include "ofp_header.h"

namespace codec {

class ofp_flow_mod {
public:
  
  // pg 63 v1.4.1 s7.2.2 (a default empty entry)
  struct ofp_match_: public ofp141::ofp_match {
    void init() {
      type = ofp141::ofp_match_type::OFPMT_OXM;
      length = sizeof( type ) + sizeof( length );
    }
  };
  
  // pg 74 v1.4.1 s7.2.3 (a default empty entry)
  struct ofp_instruction_header_ : public ofp141::ofp_instruction_header {
    void init() {
      type = ofp141::ofp_instruction_type::OFPIT_APPLY_ACTIONS;
      len = sizeof( ofp_instruction_header_ );
    }
  };
    

  // pg 77 v1.4.1 s7.2.4
  struct ofp_action_output_: public ofp141::ofp_action_output {
    void init() {
      type = ofp141::ofp_action_type::OFPAT_OUTPUT;
      len = sizeof( ofp141::ofp_action_output );
      port = ofp141::ofp_port_no::OFPP_CONTROLLER;
      max_len = ofp141::ofp_controller_max_len::OFPCML_NO_BUFFER;
    }
  };
  
  // pg 75 v1.4.1 s7.2.3
  struct ofp_instruction_actions_: public ofp141::ofp_instruction_actions {
    void init() {
      type = ofp141::ofp_instruction_type::OFPIT_APPLY_ACTIONS;
      len = sizeof( ofp141::ofp_instruction_actions ); // needs to be updated with added actions
    }
  };
    
  // pg 89 v1.4.1 s7.3.4.2
  struct ofp_flow_mod_: public ofp141::ofp_flow_mod {
    void init() {
      auto p = new( &header ) codec::ofp_header::ofp_header_;
      p->init();
      p->type = ofp141::ofp_type::OFPT_FLOW_MOD;
      codec::ofp_header::NewXid( *p );
      cookie = 0;
      cookie_mask = 0;
      table_id = 0;
      command = 0;
      idle_timeout = 0;
      hard_timeout = 0;
      priority = 0;
      buffer_id = OFP_NO_BUFFER;
      out_port = ofp141::ofp_port_no::OFPP_ANY;
      out_group = ofp141::ofp_group::OFPG_ANY;
      flags = 
        //ofp141::ofp_flow_mod_flags::OFPFF_SEND_FLOW_REM |
        //ofp141::ofp_flow_mod_flags::OFPFF_CHECK_OVERLAP
        0
        ;
      importance = 0;
    }
  };
  
  ofp_flow_mod( );
  virtual ~ofp_flow_mod( );
private:

};

} // namespace codec

#endif /* OFP_FLOW_MOD_H */

