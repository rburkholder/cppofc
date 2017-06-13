/* 
 * File:   ofp_async_config.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on June 8, 2017, 9:20 AM
 */

#ifndef OFP_ASCYNC_CONFIG_H
#define OFP_ASCYNC_CONFIG_H

#include <functional>

#include "../openflow/openflow-spec1.4.1.h"

#include "../common.h"

namespace codec {
  
  // controller states described in section 6.3.7, page 40, v1.4.1

class ofp_async_config {
  const ofp141::ofp_async_config& m_packet;
public:
  ofp_async_config( const ofp141::ofp_async_config& );
  virtual ~ofp_async_config( );
private:
  typedef std::function<void( const ofp141::ofp_async_config_prop_header&)> funcPropertyHeader_t;
  
  void Properties( funcPropertyHeader_t f ) const;
  void Decode( const ofp141::ofp_async_config_prop_header& );
};

} // namespace codec

#endif /* OFP_ASCYNC_CONFIG_H */

