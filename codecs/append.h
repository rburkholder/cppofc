/*
 * File:   append.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on December 16, 2018, 3:03 PM
 */

#ifndef OFP_APPEND_H
#define OFP_APPEND_H

namespace ofp {
  // TODO: move this out to common?
  template<typename T>
  T* Append( vByte_t& v ) {
    size_t increment( sizeof( T ) );
    size_t placeholder( v.size() );
    size_t new_size = placeholder + increment;
    v.resize( new_size );
    T* p = new( v.data() + placeholder ) T;
    return p;
  }
} // namespace ofp

#endif /* OFP_APPEND_H */

