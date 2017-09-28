/* 
 * File:   ofp_packet_out.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on June 13, 2017, 4:18 PM
 */

#ifndef OFP_PACKET_OUT_H
#define OFP_PACKET_OUT_H

 #include <cstring>

#include "../openflow/openflow-spec1.4.1.h"

#include "../common.h"
#include "../codecs/ofp_header.h"

namespace codec {

class ofp_packet_out { // page 129 v1.4.1 s7.3.6
public:
  
  struct ofp_action_output_: public ofp141::ofp_action_output {
    ofp_action_output_() {}
    void init( 
      uint32_t port_ = ofp141::ofp_port_no::OFPP_NORMAL, 
      uint16_t max_len_ = ofp141::ofp_controller_max_len::OFPCML_NO_BUFFER
    ) {
      type = ofp141::ofp_action_type::OFPAT_OUTPUT;
      assert( sizeof( ofp_action_output_ ) == sizeof( ofp141::ofp_action_output ) );
      len = sizeof( ofp_action_output_ );
      port = port_;
      max_len = max_len_;
      std::memset( pad, 0, 6 );
    }
    //size_t size() const { return sizeof( this ); }
  };
  
  struct ofp_packet_out_: public ofp141::ofp_packet_out {
    ofp_packet_out_() {}
    void init( uint32_t size, uint32_t in_port_, uint32_t buffer_id_ = OFP_NO_BUFFER ) {
      auto* pHeader = new ( &header ) codec::ofp_header::ofp_header_;  // need to change header structure
      pHeader->init();
      header.type = ofp141::ofp_type::OFPT_PACKET_OUT;
      header.length = size;
      buffer_id = buffer_id_;
      in_port = in_port_;
      assert( sizeof( ofp_packet_out_ ) == sizeof( ofp141::ofp_packet_out ) );
      actions_len = sizeof( ofp_action_output_ );
      std::memset( pad, 0, 6 );
    }
    //size_t size() const {
    //  return 0; // need to add up sub-structures
    //}
  };
  
  ofp_packet_out();
  virtual ~ofp_packet_out( );
  void build( vByte_t&, uint32_t nPort, size_t nOctets, void* pSrc );
private:

};

} // namespace codec

//struct ofp_packet_out {
//    struct ofp_header header;
//    big_uint32_t buffer_id;           /* ID assigned by datapath (OFP_NO_BUFFER
//                                     if none). */
//    big_uint32_t in_port;             /* Packet's input port or OFPP_CONTROLLER. */
//    big_uint16_t actions_len;         /* Size of action array in bytes. */
//    big_uint8_t pad[6];
//    struct ofp_action_header actions[0]; /* Action list - 0 or more. */
    /* The variable size action list is optionally followed by packet data.
     * This data is only present and meaningful if buffer_id == -1. */
    /* big_uint8_t data[0]; */        /* Packet data.  The length is inferred
                                     from the length field in the header. */
//};
//OFP_ASSERT(sizeof(struct ofp_packet_out) == 24);

//struct ofp_action_output {
//    big_uint16_t type;                  /* OFPAT_OUTPUT. */
//    big_uint16_t len;                   /* Length is 16. */
//    big_uint32_t port;                  /* Output port. */
//    big_uint16_t max_len;               /* Max length to send to controller. */
//    big_uint8_t pad[6];                 /* Pad to 64 bits. */
//};
//OFP_ASSERT(sizeof(struct ofp_action_output) == 16);

#endif /* OFP_PACKET_OUT_H */

