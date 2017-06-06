/* 
 * File:   ofp_features_request.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on June 6, 2017, 10:21 AM
 */

#ifndef OFP_FEATURES_REQUEST_H
#define OFP_FEATURES_REQUEST_H

#include "../openflow/openflow-spec1.4.1.h"

namespace codec {

// page 83 of v1.4.1
class ofp_switch_features {
  const ofp141::ofp_switch_features& m_packet;
public:
  ofp_switch_features( const ofp141::ofp_switch_features& packet );
  virtual ~ofp_switch_features( );
private:

};

} // namespace codec

#endif /* OFP_FEATURES_REQUEST_H */

