/* 
 * File:   ofp_hello.h
 * Author: vagrant
 *
 * Created on May 16, 2017, 4:40 PM
 */

#ifndef OFP_HELLO_H
#define OFP_HELLO_H

#include <functional>

#include "../openflow/openflow-spec1.4.1.h"

namespace codec {

class ofp_hello {
  const ofp141::ofp_hello& m_packet;
public:
  ofp_hello( const ofp141::ofp_hello& packet );
  virtual ~ofp_hello();
  
  auto Version() const { return m_packet.header.version; }
  

  bool Supported( uint8_t version ) const;

private:
  typedef std::function<void(const ofp141::ofp_hello_elem_header&)> funcElemeHeader_t;

  uint64_t m_bitmap;

  void Elements( funcElemeHeader_t f ) const;
  void Decode( const ofp141::ofp_hello_elem_header& );
};

} // namespace codec

#endif /* OFP_HELLO_H */

