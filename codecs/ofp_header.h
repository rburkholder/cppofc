/*
 * File:   ofp_header.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on June 5, 2017, 10:43 AM
 */

#ifndef OFP_HEADER_H
#define OFP_HEADER_H

#include <cstdint>

#include <boost/endian/arithmetic.hpp>

#include "../openflow/openflow-spec1.4.1.h"

// page 56 of version 1.4.1
namespace codec {
namespace ofp_header {

struct ofp_header_ {
  boost::endian::big_uint8_t version;
  boost::endian::big_uint8_t type;
  boost::endian::big_uint16_t length;
  boost::endian::big_uint32_t xid;
  void init() {
    version = OFP_VERSION;
    type = ofp141::ofp_type::OFPT_HELLO; // will be overwritten with other types
    length = sizeof( ofp_header_ ); // overwritten in other types
    xid = 0;  // needs to be copied from request, or dynamically incremented
  }
};
OFP_ASSERT(sizeof(ofp_header_) == 8);

void NewXid( ofp_header_& );
void CopyXid( const ofp_header_&, ofp_header_& );

} // namespace ofp_header
} // namespace codec

#endif /* OFP_HEADER_H */

