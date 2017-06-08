/* 
 * File:   ofp_hello.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on May 16, 2017, 4:40 PM
 */

#ifndef OFP_HELLO_H
#define OFP_HELLO_H

#include <functional>
#include <memory>
#include <array>

#include <boost/asio/buffer.hpp>
#include <boost/endian/arithmetic.hpp>

#include "../openflow/openflow-spec1.4.1.h"

#include "common.h"
#include "ofp_header.h"

namespace codec {

// page 147 of v1.4.1
class ofp_hello {
  const ofp141::ofp_hello& m_packet;
public:

  struct ofp_hello_elem_versionbitmap {
    typedef boost::endian::big_uint32_t bitmap_t;
    boost::endian::big_uint16_t type;
    boost::endian::big_uint16_t length;
    bitmap_t bitmap[ 1 ];
    // will need more bytes when bitmap size > 1
    void init( void ) {
      type = ofp141::OFPHET_VERSIONBITMAP;
      length = sizeof( ofp_hello_elem_versionbitmap );
      bitmap[ 0 ] = 1 << OFP_VERSION;
    }
  };
  
  struct ofp_hello_ {
    ofp_header::ofp_header_ header;
    ofp_hello_elem_versionbitmap elem_versionbitmap;
    void init() {
      header.init();
      elem_versionbitmap.init();
      header.length = sizeof( ofp_hello_ );
    }
  };

  typedef std::shared_ptr<const ofp_hello_> ofp_hello_ptr;
  
  ofp_hello( const ofp141::ofp_hello& packet );
  virtual ~ofp_hello();
  
  auto Version() const { return m_packet.header.version; }

  bool Supported( uint8_t version ) const;
  
  static vChar_t Create( vChar_t v);

private:
  
  typedef std::function<void(const ofp141::ofp_hello_elem_header&)> funcElementHeader_t;

  uint32_t m_bitmap;
  
  void Elements( funcElementHeader_t f ) const;
  void Decode( const ofp141::ofp_hello_elem_header& );
  //void Encode( std::array)
};

} // namespace codec

#endif /* OFP_HELLO_H */

