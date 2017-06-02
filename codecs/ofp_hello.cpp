/* 
 * File:   ofp_hello.cpp
 * Author: vagrant
 * 
 * Created on May 16, 2017, 4:40 PM
 */

#include <iostream>

#include "ofp_hello.h"

namespace codec {

ofp_hello::ofp_hello( const ofp141::ofp_hello& packet )
: m_packet( packet ), m_bitmap( 0 )
{
  std::cout << "hello: " << packet.header.length << "," << sizeof( ofp141::ofp_header ) << std::endl;
  Elements( [this]( const ofp141::ofp_hello_elem_header& element ) { Decode( element ); } );
}

ofp_hello::~ofp_hello() {
}

void ofp_hello::Elements( funcElemeHeader_t f ) const {
  auto end = (uint8_t*)m_packet.elements + m_packet.header.length - sizeof( ofp141::ofp_header );
  auto   p = (uint8_t*)m_packet.elements;
  while ( p < end ) {
    auto p2 = (ofp141::ofp_hello_elem_header*) p;
    f( *p2 );
    p += p2->length; // standard says exclusive of padding, so not sure what to do 
  }
}

void ofp_hello::Decode(const ofp141::ofp_hello_elem_header& element ) {
  switch ( element.type ) {
    case ofp141::OFPHET_VERSIONBITMAP:
      auto& elem_versionbitmap( (const ofp141::ofp_hello_elem_versionbitmap&) element );
      auto nBytes = elem_versionbitmap.length - 4;
      //auto nAllZero = (elem_versionbitmap.length + 7)/8*8 - elem_versionbitmap.length;
      m_bitmap = {};
      auto p = elem_versionbitmap.bitmaps;
      for ( size_t ix = 0; ix < nBytes/4 && ix < 2; ix++ ) {
        switch ( ix ) {
          case 0: 
            m_bitmap |= (uint64_t)( p[ix] );
            break;
          case 1:
            m_bitmap |= (uint64_t)( p[ix] ) << 32;
            break;
        }
      }
      auto value = OFP_VERSION;  // this is going to cause problems with multiple versioned headers
      if ( !Supported( value ) ) {
        throw std::runtime_error( "Requires version 1.4.1: " );
      }
      
  }
}

bool ofp_hello::Supported( uint8_t version ) const {
  uint64_t temp = 1;
  return 0 < ( temp << version );
}

} // namespace codec