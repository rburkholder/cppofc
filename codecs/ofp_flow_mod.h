/* 
 * File:   ofp_flow_mod.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on June 8, 2017, 1:47 PM
 */

#ifndef OFP_FLOW_MOD_H
#define OFP_FLOW_MOD_H

#include <cstring>
#include <iostream>
#include <cassert>
#include <functional>
#include <tuple>

#include "../openflow/openflow-spec1.4.1.h"
#include "../common.h"
#include "ofp_header.h"

namespace codec {

class ofp_flow_mod {
public:
  
  typedef std::function<void( uint32_t )> fInPort_t;
  typedef std::function<void( mac_t& )> fEthDest_t;
  typedef std::function<void( mac_t& )> fEthSrc_t;
  typedef std::tuple< // need to rework this, duplicate signatures won't work
    fInPort_t
//    fEthDest_t
//    fEthSrc_t
      > rfMatch_t;
  
  // included in oxm fields.
  struct oxm_header_ { // convert to OXM_CLASS, OXM_FIELD, OXM_TYPE, ... ?
    boost::endian::big_uint16_t m_class;
    boost::endian::big_uint8_t m_mixed;
    boost::endian::big_uint8_t m_length;
    uint8_t field[0];
    
    uint16_t oxm_class() { return m_class; }
    uint8_t oxm_field() { return (uint8_t)m_mixed >> 1; }
    bool oxm_hasmask() { return (uint8_t)m_mixed && 0x01; }
    uint8_t oxm_length() { return m_length; }
  };
  
  struct ofpxmt_ofb_in_port_ {
    //oxm_header_ header;
    boost::endian::big_uint32_t header;
    boost::endian::big_uint32_t port;
    
    void init( uint32_t port_ ) {
      header = OXM_HEADER( 
        ofp141::ofp_oxm_class::OFPXMC_OPENFLOW_BASIC,
        ofp141::oxm_ofb_match_fields::OFPXMT_OFB_IN_PORT, 
        4 
        );
      port = port_;
    }
  };
  
  struct ofpxmt_ofb_eth_ {
    //oxm_header_ header;
    boost::endian::big_uint32_t header;
    mac_t mac;
    
    void init( ofp141::oxm_ofb_match_fields field, const mac_t& mac_ ) {
      assert( 
        ( ofp141::oxm_ofb_match_fields::OFPXMT_OFB_ETH_DST == field ) ||
        ( ofp141::oxm_ofb_match_fields::OFPXMT_OFB_ETH_SRC == field ) ||
        ( ofp141::oxm_ofb_match_fields::OFPXMT_OFB_ARP_SHA == field ) ||
        ( ofp141::oxm_ofb_match_fields::OFPXMT_OFB_ARP_THA == field )
        );
      header = OXM_HEADER(ofp141::ofp_oxm_class::OFPXMC_OPENFLOW_BASIC, field, 6 );
      std::memcpy( mac, mac_, sizeof( mac_t ) );
    }
  };
  
  // pg 63 v1.4.1 s7.2.2 (a default empty entry)
  struct ofp_match_: public ofp141::ofp_match {
    void init() {
      type = ofp141::ofp_match_type::OFPMT_OXM;
      length = sizeof( type ) + sizeof( length );
    }
    size_t fill_size() const { return ( length + 7 ) / 8 * 8 - length; }
    uint8_t* fill() {  // returns next available octet location
      auto* pPad = new( this ) uint8_t;
      pPad += length;
      uint8_t cnt = fill_size();
      while ( 0 != cnt ) {
       *pPad = 0;
        pPad++;
        cnt--;
      }
      return pPad;
    }
    
    void decode( rfMatch_t& rfMatch ) {
      const uint16_t lenMatches( length - 4 );
      uint16_t cnt( 0 );
      oxm_header_* p; 
      while ( lenMatches > cnt ) {
        p = new( &ofp141::ofp_match::oxm_fields[cnt] ) oxm_header_;
        std::cout 
          << "ofp_match: "
          << "class=" << std::hex << p->oxm_class() << std::dec
          << ",field=" << (uint16_t)p->oxm_field()
          << ",hasmask=" << p->oxm_hasmask()
          << ",length=" << (uint16_t)p->oxm_length()
          << std::endl;
        switch ( p->oxm_field() ) {
          case ofp141::OFPXMT_OFB_IN_PORT: {
            assert( sizeof(uint32_t) == p->oxm_length() );
            ofpxmt_ofb_in_port_* pInPort = new( p ) ofpxmt_ofb_in_port_;
            std::cout << "in_port=" << pInPort->port << std::endl;
            if ( nullptr != std::get<fInPort_t>( rfMatch ) ) {
              std::get<fInPort_t>( rfMatch )( pInPort->port );
            } // if
            } // case
            break;
          case ofp141::OFPXMT_OFB_ETH_DST: {
            assert( 6 == p->oxm_length() );
            ofpxmt_ofb_eth_* pMac = new( p ) ofpxmt_ofb_eth_;
            //if ( nullptr != std::get<fEthDest_t>( rfMatch ) ) {
            //  std::get<fEthDest_t>( rfMatch )( pMac->mac );
            //}
            }
          case ofp141::OFPXMT_OFB_ETH_SRC: {
            assert( 6 == p->oxm_length() );
            ofpxmt_ofb_eth_* pMac = new( p ) ofpxmt_ofb_eth_;
            //if ( nullptr != std::get<fEthSrc_t>( rfMatch ) ) {
            //  std::get<fEthSrc_t>( rfMatch )( pMac->mac );
            //}
            }
          default:
            std::cout << "**** ofp_match: unknown field" << std::endl;
        }
        cnt += sizeof( oxm_header_ ) + p->oxm_length();
        assert( lenMatches >= cnt );
      }
    }
  };
  
  // pg 74 v1.4.1 s7.2.3 (a default empty entry)
  struct ofp_instruction_header_ : public ofp141::ofp_instruction_header {
    void init() {
      type = ofp141::ofp_instruction_type::OFPIT_APPLY_ACTIONS;
      len = sizeof( ofp_instruction_header_ );
    }
  };
    

  // pg 77 v1.4.1 s7.2.4
  struct ofp_action_output_: public ofp141::ofp_action_output {
    void init( uint32_t nPort = ofp141::ofp_port_no::OFPP_CONTROLLER ) {
      type = ofp141::ofp_action_type::OFPAT_OUTPUT;
      len = sizeof( ofp141::ofp_action_output );
      port = nPort;
      max_len = ofp141::ofp_controller_max_len::OFPCML_NO_BUFFER;
      std::memset( pad, 0, 6 );
    }
  };
  
  // pg 75 v1.4.1 s7.2.3
  struct ofp_instruction_actions_: public ofp141::ofp_instruction_actions {
    void init() {
      type = ofp141::ofp_instruction_type::OFPIT_APPLY_ACTIONS;
      len = sizeof( ofp141::ofp_instruction_actions ); // needs to be updated with added actions
    }
  };
    
  // pg 89 v1.4.1 s7.3.4.2
  struct ofp_flow_mod_: public ofp141::ofp_flow_mod {
    void init() {
      auto pHeader = new( &header ) codec::ofp_header::ofp_header_;
      pHeader->init();
      header.type = ofp141::ofp_type::OFPT_FLOW_MOD;
      assert( sizeof( ofp_flow_mod_ ) == sizeof( ofp141::ofp_flow_mod ) );
      header.length = sizeof( ofp_flow_mod_ ) - sizeof( match );
      codec::ofp_header::NewXid( *pHeader );
      cookie = 0;
      cookie_mask = 0;
      table_id = 0;
      command = ofp141::ofp_flow_mod_command::OFPFC_ADD;
      idle_timeout = 0;
      hard_timeout = 0;
      priority = 0;
      buffer_id = OFP_NO_BUFFER;
      out_port = ofp141::ofp_port_no::OFPP_ANY;
      out_group = ofp141::ofp_group::OFPG_ANY;
      flags = 
        //ofp141::ofp_flow_mod_flags::OFPFF_SEND_FLOW_REM |
        //ofp141::ofp_flow_mod_flags::OFPFF_CHECK_OVERLAP
        0
        ;
      importance = 0;
      auto pMatch = new( &match ) codec::ofp_flow_mod::ofp_match_;
      pMatch->init();
    }
    
    uint8_t* fill( uint8_t& fill_size ) {  // returns next available octet location
      auto pMatch = new( &match ) codec::ofp_flow_mod::ofp_match_;
      fill_size = pMatch->fill_size();
      return pMatch->fill();
    }
  };
  
private:

};

} // namespace codec

#endif /* OFP_FLOW_MOD_H */

