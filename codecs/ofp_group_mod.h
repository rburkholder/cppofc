/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   ofp_group_mod.h
 * Author: root
 *
 * Created on December 3, 2018, 11:05 PM
 */

#ifndef OFP_GROUP_MOD_H
#define OFP_GROUP_MOD_H

#include "../openflow/openflow-spec1.4.1.h"
#include "ofp_header.h"


namespace codec {
namespace ofp_group_mod {

struct ofp_group_mod_: public ofp141::ofp_group_mod {
  void init( ofp141::ofp_group_mod_command cmd, uint32_t group_id_ ) {
    auto pHeader = new( &header ) codec::ofp_header::ofp_header_;
    pHeader->init();
    header.type = ofp141::ofp_type::OFPT_GROUP_MOD;
    header.length = sizeof( ofp_group_mod );
    assert( sizeof( ofp_group_mod_ ) == sizeof( ofp141::ofp_group_mod ) );
    //command = ofp141::ofp_group_mod_command::OFPGC_MODIFY;
    command = cmd;
    pad = 0;
    type = ofp141::ofp_group_type::OFPGT_ALL; // pg 23, v141
    group_id = group_id_;
  }
};

struct ofp_bucket_: public ofp141::ofp_bucket {
  void init() {
    assert( sizeof( ofp_bucket_ ) == sizeof( ofp141::ofp_bucket ) );
    len = sizeof( ofp_bucket_ );
    weight = 1;
    watch_port = 0;
    watch_group = 0;
    memset( pad, 0, 4 );
  }
};

} // namespace ofp_group_mod
} // namespace codec

#endif /* OFP_GROUP_MOD_H */

