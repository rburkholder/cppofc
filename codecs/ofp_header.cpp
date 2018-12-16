/*
 * File:   ofp_header.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on June 5, 2017, 1:13 PM
 */

#include "ofp_header.h"

namespace codec {
namespace ofp_header {

uint32_t xid {};

void NewXid( ofp_header_& header ) {
  xid++;
  header.xid = xid;
}

void CopyXid( const ofp_header_& src, ofp_header_& dst ) {
  dst.xid = src.xid;
}

} // namespace ofp_header
} // namespace codec