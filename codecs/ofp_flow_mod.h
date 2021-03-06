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
#include "../protocol/ethernet/address.h"
#include "ofp_header.h"

namespace codec {
namespace ofp_flow_mod {

  typedef protocol::ethernet::address_t mac_t;

  typedef std::function<void( uint32_t )> fInPortCookie_t; // in_port
  //typedef std::function<void( mac_t& )> fEth_t;

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
      header = OXM_HEADER( // line 686
        ofp141::ofp_oxm_class::OFPXMC_OPENFLOW_BASIC,
        ofp141::oxm_ofb_match_fields::OFPXMT_OFB_IN_PORT,
        4
        );
      port = port_;
    }
  };

  struct ofpxmt_ofb_vlan_vid_ {
    boost::endian::big_uint32_t header;
    boost::endian::big_uint16_t vlan;
    // will require 6 bytes of padding
    void init( uint16_t vlan_ ) {
      header = OXM_HEADER( // line 760
        ofp141::ofp_oxm_class::OFPXMC_OPENFLOW_BASIC,
        ofp141::oxm_ofb_match_fields::OFPXMT_OFB_VLAN_VID,
        2
        );
      vlan = vlan_ | ofp141::ofp_vlan_id::OFPVID_PRESENT;
    }
    void init() {
      header = OXM_HEADER( // line 760
        ofp141::ofp_oxm_class::OFPXMC_OPENFLOW_BASIC,
        ofp141::oxm_ofb_match_fields::OFPXMT_OFB_VLAN_VID,
        2
        );
      vlan = ofp141::ofp_vlan_id::OFPVID_NONE;
    }
  };

  struct ofpxmt_ofb_eth_mac_ {
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

  struct ofpxmt_ofb_eth_type_ {
    boost::endian::big_uint32_t header;
    boost::endian::big_uint16_t ethernet_type;

    void init( uint16_t ethernet_type_ ) {
      header = OXM_HEADER(ofp141::ofp_oxm_class::OFPXMC_OPENFLOW_BASIC,
      ofp141::oxm_ofb_match_fields::OFPXMT_OFB_ETH_TYPE,
      2 );
      ethernet_type = ethernet_type_;
    }
  };

  struct ofpxmt_ofb_ip_proto_ {
    boost::endian::big_uint32_t header;
    uint8_t protocol;

    void init( uint8_t protocol_ ) {
      header = OXM_HEADER(ofp141::ofp_oxm_class::OFPXMC_OPENFLOW_BASIC,
      ofp141::oxm_ofb_match_fields::OFPXMT_OFB_IP_PROTO,
      1 );
      protocol = protocol_;
    }
  };

  struct ofpxmt_ofb_port_ {
    boost::endian::big_uint32_t header;
    boost::endian::big_uint16_t port;

    void init( ofp141::oxm_ofb_match_fields field, uint16_t port_ ) {
      assert(
        ( ofp141::oxm_ofb_match_fields::OFPXMT_OFB_UDP_DST == field ) ||
        ( ofp141::oxm_ofb_match_fields::OFPXMT_OFB_UDP_SRC == field ) ||
        ( ofp141::oxm_ofb_match_fields::OFPXMT_OFB_TCP_DST == field ) ||
        ( ofp141::oxm_ofb_match_fields::OFPXMT_OFB_TCP_SRC == field )
        );
      header = OXM_HEADER(ofp141::ofp_oxm_class::OFPXMC_OPENFLOW_BASIC,
      field,
      2 );
      port = port_;
    }
  };

  struct ofpxmt_ofb_metadata_ {
    boost::endian::big_uint32_t header;
    boost::endian::big_uint64_t metadata;

    void init( uint32_t metadata_ ) {
      header = OXM_HEADER(ofp141::ofp_oxm_class::OFPXMC_OPENFLOW_BASIC,
      ofp141::oxm_ofb_match_fields::OFPXMT_OFB_METADATA,
      8 );
      metadata = metadata_;
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

    // TODO: will need to refactor as different matches are required
    // decodes *this ofp_match structure
    // 2018/12/08, really only need IN_PORT for now
    void decode( fInPortCookie_t fDecode ) {
      const uint16_t lenMatches( length - 4 );
      uint16_t cnt( 0 );
      oxm_header_* p;
      bool bFoundInPort( false );
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
            if ( nullptr != fDecode ) {
              fDecode( pInPort->port );
            } // if
            bFoundInPort = true;
            } // case
            break;
          case ofp141::OFPXMT_OFB_ETH_DST: {
            assert( 6 == p->oxm_length() );
            ofpxmt_ofb_eth_mac_* pMac = new( p ) ofpxmt_ofb_eth_mac_;
            //if ( nullptr != std::get<fEthDest_t>( rfMatch ) ) {
            //  std::get<fEthDest_t>( rfMatch )( pMac->mac );
            //}
            }
          case ofp141::OFPXMT_OFB_ETH_SRC: {
            assert( 6 == p->oxm_length() );
            ofpxmt_ofb_eth_mac_* pMac = new( p ) ofpxmt_ofb_eth_mac_;
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
    assert( bFoundInPort );
    }
  };

  // pg 74 v1.4.1 s7.2.3 (a default empty entry)
  struct ofp_instruction_header_ : public ofp141::ofp_instruction_header {
    void init() {
      type = ofp141::ofp_instruction_type::OFPIT_APPLY_ACTIONS;
      len = sizeof( ofp_instruction_header_ );
    }
  };

  // pg 80, v1.4.1 s7.2.4
  struct ofp_action_pop_vlan_: public ofp141::ofp_action_generic {
    void init() {
      type = ofp141::ofp_action_type::OFPAT_POP_VLAN;
      len = sizeof( ofp141::ofp_action_generic );
      std::memset( pad, 0, 4 );
    }
  };

  // pg 79, v1.4.1 s7.2.4
  struct ofp_action_push_vlan_: public ofp141::ofp_action_push {
    void init( uint16_t ethertype_ ) {
      type = ofp141::ofp_action_type::OFPAT_PUSH_VLAN;
      len = sizeof( ofp141::ofp_action_push );
      assert( ( 0x8100 == ethertype_ ) || ( 0x88a8 == ethertype_ ) );
      ethertype = ethertype_;
      std::memset( pad, 0, 2 );
    }
  };

  // pg 80, v1.4.1 s7.2.4
  struct ofp_action_set_field_: public ofp141::ofp_action_set_field {
    void init() {
      type = ofp141::ofp_action_type::OFPAT_SET_FIELD;
      len  = sizeof( ofp141::ofp_action_set_field );
    }
    uint8_t* tlv() { return (uint8_t*)(&field[0]); }
  };

  struct ofp_action_set_field_vlan_id_: public ofp141::ofp_action_set_field {
    static const size_t placeholding = sizeof( ofpxmt_ofb_vlan_vid_ ) - sizeof( field );
    static const size_t padding
      = 16 -
        ( sizeof( ofp141::ofp_action_set_field ) + placeholding );
    uint8_t placeholder[ placeholding ];
    uint8_t pad[ padding ];
    void init( uint16_t idVlan ) {
      assert( 16 == sizeof( ofp_action_set_field_vlan_id_ ) );
      auto pVid = new( &(field[0]) ) ofpxmt_ofb_vlan_vid_;
      pVid->init( idVlan );
      std::memset( pad, 0, padding );
      type = ofp141::ofp_action_type::OFPAT_SET_FIELD;
      len = 16;
    }
  };

  struct ofp_action_set_field_metadata_: public ofp141::ofp_action_set_field {
    typedef ofpxmt_ofb_metadata_ ofpxmt;
    static const size_t placeholding = sizeof( ofpxmt ) - sizeof( field );
    static const size_t padding
      = 16 -
        ( sizeof( ofp141::ofp_action_set_field ) + placeholding );
    //assert( 0 == padding );
    uint8_t placeholder[ placeholding ];
    //uint8_t pad[ padding ];
    void init( uint64_t metadata ) {
      assert( 16 == sizeof( ofp_action_set_field_metadata_ ) );
      auto pMeta = new( &(field[0]) ) ofpxmt;
      pMeta->init( metadata );
      //std::memset( pad, 0, padding );
      type = ofp141::ofp_action_type::OFPAT_SET_FIELD;
      len = 16;
    }
  };

  // pg 77 v1.4.1 s7.2.4
  struct ofp_action_output_: public ofp141::ofp_action_output {
    void init( uint32_t nPort = ofp141::ofp_port_no::OFPP_CONTROLLER ) {
      type = ofp141::ofp_action_type::OFPAT_OUTPUT;
      len = sizeof( ofp141::ofp_action_output );
      port = nPort;
      //max_len = ofp141::ofp_controller_max_len::OFPCML_NO_BUFFER;
      max_len = 0;
      std::memset( pad, 0, 6 );
    }
  };

  // pg 74, v1.4.1 s7.2.3
  struct ofp_instructions_goto_table: public ofp141::ofp_instruction_goto_table {
    void init( uint8_t idTable ) {
      type = ofp141::ofp_instruction_type::OFPIT_GOTO_TABLE;
      len = sizeof( ofp141::ofp_instruction_goto_table );
      table_id = idTable;
      memset( pad, 0, 3 );
    }
  };

  // pg 75 v1.4.1 s7.2.3
  struct ofp_instruction_actions_: public ofp141::ofp_instruction_actions {
    void init( ofp141::ofp_instruction_type type_ = ofp141::ofp_instruction_type::OFPIT_APPLY_ACTIONS ) {
    //void init( ofp141::ofp_instruction_type type_ = ofp141::ofp_instruction_type::OFPIT_WRITE_ACTIONS ) {
      type = type_;
      len = sizeof( ofp141::ofp_instruction_actions ); // needs to be updated with added actions
      memset( pad, 0, 4 );
      // specific actions to follow this structure
    }
  };

  // pg 89 v1.4.1 s7.3.4.2
  struct ofp_flow_mod_: public ofp141::ofp_flow_mod { // line 1379 in ofp141 file
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
      auto pMatch = new( &match ) codec::ofp_flow_mod::ofp_match_;  // line 594
      pMatch->init();
    }

    uint8_t* fill( uint8_t& fill_size ) {  // returns next available octet location
      auto pMatch = new( &match ) codec::ofp_flow_mod::ofp_match_;
      fill_size = pMatch->fill_size();
      return pMatch->fill();
    }
  };

} // namespace ofp_flow_mod
} // namespace codec

#endif /* OFP_FLOW_MOD_H */

