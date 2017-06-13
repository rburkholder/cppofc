/* 
 * File:   ofp_hello.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 * 
 * Created on May 16, 2017, 4:40 PM
 */

#include <iostream>

#include "ofp_hello.h"

namespace codec {
  
ofp_hello::ofp_hello( const ofp141::ofp_hello& packet )
: m_packet( packet ), m_bitmap( 0 )
{
  std::cout << "ofp_hello: " << packet.header.length << "," << sizeof( ofp141::ofp_header ) << std::endl;
  Elements( [this]( const ofp141::ofp_hello_elem_header& element ) { Decode( element ); } );
}

ofp_hello::~ofp_hello() {
}

void ofp_hello::Elements( funcElementHeader_t f ) const {
  auto   p = (uint8_t*)m_packet.elements;
  auto end = (uint8_t*)m_packet.elements + m_packet.header.length - sizeof( ofp141::ofp_header );
  while ( p < end ) {
    auto p2 = (ofp141::ofp_hello_elem_header*) p;
    f( *p2 );
    p += p2->length; // standard says exclusive of padding, so not sure what to do 
  }
}

void ofp_hello::Decode(const ofp141::ofp_hello_elem_header& element ) {
  // TODO:  logic in this whole thing needs to be redone for flexibility
  switch ( element.type ) {
    case ofp141::OFPHET_VERSIONBITMAP:
      auto& elem_versionbitmap( (const ofp141::ofp_hello_elem_versionbitmap&) element );
      auto nBytes = elem_versionbitmap.length - 4;
      //auto nAllZero = (elem_versionbitmap.length + 7)/8*8 - elem_versionbitmap.length;
      m_bitmap = {};
      auto p = elem_versionbitmap.bitmaps;
      if ( sizeof( uint32_t) <= nBytes ) {
        m_bitmap = *p;
        auto value = OFP_VERSION;  // this is going to cause problems with multiple versioned headers
        if ( !Supported( value ) ) {
          throw std::runtime_error( "Requires version 1.4.1: " );
        }
      }
      else {
        throw std::runtime_error( "no bitmap found" );
      }
  }
}

bool ofp_hello::Supported( uint8_t version ) const {
  uint32_t temp = 1;
  auto result = ( temp << version ) & m_bitmap;
  return 0 < result;
}

vByte_t ofp_hello::Create( vByte_t v ) {
  v.resize( sizeof( ofp_hello_ ) );
  auto* p = new( v.data() ) ofp_hello_;
  p->init();
  return v;
}

} // namespace codec