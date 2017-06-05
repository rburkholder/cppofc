/* 
 * File:   ofp_header.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on June 5, 2017, 1:13 PM
 */

#include "ofp_header.h"

namespace codec {
  
uint32_t ofp_header::m_xid = 0;

void ofp_header::NewXid( ofp_header_& header ) {
  m_xid++;
  header.xid = m_xid;
}

void ofp_header::CopyXid( const ofp_header_& src, ofp_header_& dst ) {
  dst.xid = src.xid;
}

} // namespace codec