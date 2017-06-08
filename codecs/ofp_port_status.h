/* 
 * File:   ofp_port_status.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on June 8, 2017, 11:12 AM
 */

#ifndef OFP_PORT_STATUS_H
#define OFP_PORT_STATUS_H

#include "../openflow/openflow-spec1.4.1.h"

namespace codec {

// page 144 of v1.4.1
class ofp_port_status {
  const ofp141::ofp_port_status& m_packet;
public:
  ofp_port_status( const ofp141::ofp_port_status& );
  virtual ~ofp_port_status( );
private:

};

} // namespace

#endif /* OFP_PORT_STATUS_H */

