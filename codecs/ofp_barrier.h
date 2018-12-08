/*
 * File:   ofp_barrier.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on December 7, 2018, 10:59 PM
 */

#ifndef OFP_BARRIER_H
#define OFP_BARRIER_H

#include "ofp_header.h"

namespace codec {
namespace ofp_barrier {

struct ofp_barrier_: public ofp141::ofp_header {
  void init() {
    auto* pHeader = new( this ) codec::ofp_header::ofp_header_;
    pHeader->init();
    ::codec::ofp_header::NewXid( *pHeader );
    pHeader->type = ofp141::ofp_type::OFPT_BARRIER_REQUEST;
  }
};


} // namespace ofp_barrier
} // namespace codec

#endif /* OFP_BARRIER_H */

