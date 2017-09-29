/* 
 * File:   tcp_session.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 * 
 * Created on June 6, 2017, 11:07 AM
 */

// 

#include <iostream>
#include <iomanip>
#include <cstring>

#include "codecs/ofp_header.h"
#include "codecs/ofp_hello.h"
#include "codecs/ofp_switch_features.h"
#include "codecs/ofp_async_config.h"
#include "codecs/ofp_port_status.h"
#include "codecs/ofp_flow_mod.h"
#include "codecs/ofp_packet_out.h"

#include "protocol/ethernet.h"

#include "common.h"
#include "hexdump.h"
#include "tcp_session.h"
#include "protocol/arp.h"

// 2017/09/28 - once the bridge works, then can start work on routing:
//   https://dtucker.co.uk/hack/building-a-router-with-openvswitch.html

// 2017/09/28 - test port up/down/add/delete messages

void tcp_session::start() {
  std::cout << "start begin: " << std::endl;
  try {
    do_read();
  }
  catch(...) {
    std::cout << "do_read issues" << std::endl;
  }
  std::cout << "start end: " << std::endl;
}

void tcp_session::do_read() {
  //std::cout << "do_read begin: " << std::endl;
  // going to need to perform serialization as bytes come in (multiple packets joined together)?
  auto self(shared_from_this());
  m_vRx.resize( max_length );
  m_socket.async_read_some(boost::asio::buffer(m_vRx),
      [this, self](boost::system::error_code ec, std::size_t length)
      {
        //std::cout << "async_read begin: " << std::endl;
        // NOTE:  multiple responses may occur in same packet, so may need to
        //   sequentially process the results
        if (!ec) {
          std::cout << ">>> total read length: " << length << std::endl;
          
          auto* pPacketIn = m_vRx.data();
          auto* pEnd = pPacketIn + length;

          while ( pPacketIn < pEnd ) {
            
            assert( sizeof( ofp141::ofp_header ) <= pEnd - pPacketIn );
            
            auto pHeader = new( pPacketIn ) ofp141::ofp_header;
            if ( pHeader->length <= pEnd - pPacketIn ) {}
            else {
              auto len = pHeader->length;
              auto diff = pEnd - pPacketIn;
              std::cout << "problem (expected,supplied): " << len << "," << diff << std::endl;
              assert( 0 );
            }

            std::cout 
              << "IN: "
              << HexDump<uint8_t*>( pPacketIn, pPacketIn + pHeader->length )
              << std::endl;
            
            std::cout << (uint16_t)pHeader->version << "," << (uint16_t)pHeader->type << "," << pHeader->length << "," << pHeader->xid << std::endl;
            if ( OFP_VERSION == pHeader->version ) {
              switch (pHeader->type) {
                case ofp141::ofp_type::OFPT_HELLO: {
                  // need to wrap following in try/catch
                  const auto pHello = new(pPacketIn) ofp141::ofp_hello;
                  codec::ofp_hello hello( *pHello );
                  // do some processing
                  //  then send hello back
                  QueueTxToWrite( std::move( codec::ofp_hello::Create( std::move( GetAvailableBuffer() ) ) ) );
                  QueueTxToWrite( std::move( codec::ofp_switch_features::CreateRequest( std::move( GetAvailableBuffer() ) ) ) );

                  struct add_table_miss_flow {
                    codec::ofp_flow_mod::ofp_flow_mod_ mod;
                    codec::ofp_flow_mod::ofp_instruction_actions_ actions;
                    codec::ofp_flow_mod::ofp_action_output_ action;
                    void init() {
                      mod.init();
                      actions.init();
                      action.init();
                      mod.header.length = sizeof( add_table_miss_flow );
                      actions.len += sizeof( action );

                      // need to update pMatch length once match fields are added                      
                    }
                  };
                  vByte_t v = std::move( GetAvailableBuffer() );
                  v.resize( sizeof( add_table_miss_flow ) );
                  auto pMod = new( v.data() ) add_table_miss_flow; 
                  pMod->init();
                  //pMod->mod.command = ofp141::ofp_flow_mod_command::OFPFC_ADD; // by default
                  //std::cout << "MissFlow: ";
                  //HexDump( std::cout, v.begin(), v.end() );
                  QueueTxToWrite( std::move( v ) );
                  }
                  break;
                case ofp141::ofp_type::OFPT_PACKET_IN: { // v1.4.1 page 140
                  
                  // rather than flood (output), re-use the table when possible
                  // now should be able to modularize this code
                  
                  const auto pPacket = new(pPacketIn) ofp141::ofp_packet_in;
                  std::cout 
                    << "packet in meta: " 
                    << "bufid=" << std::hex << pPacket->buffer_id << std::dec
                    << ", total_len=" << pPacket->total_len
                    << ", reason=" << (uint16_t)pPacket->reason
                    << ", tabid=" << (uint16_t)pPacket->table_id
                    << ", cookie=" << pPacket->cookie
                    << ", match type=" << pPacket->match.type
                    << ", match len=" << pPacket->match.length
                    << ", match=" // section 7.2.2 page 63
                    << HexDump<boost::endian::big_uint8_t*>( pPacket->match.oxm_fields, pPacket->match.oxm_fields + pPacket->match.length - 4 )
                    << ::std::endl;
                  auto pMatch = new( &pPacket->match ) codec::ofp_flow_mod::ofp_match_;
                  const auto pPayload = pPacketIn + pPacket->header.length - pPacket->total_len;
                  std::cout 
                    << "  content: "
                    << HexDump<uint8_t*>( pPayload, pPayload + pPacket->total_len )
                    << ::std::endl;
                  ethernet::header ethernet( *pPayload ); // pull out ethernet header
                  std::cout << ethernet << ::std::endl;
                  codec::ofp_flow_mod::rfMatch_t rfMatch; // probably want this init outside of loop
                  uint32_t nPort;
                  std::get<codec::ofp_flow_mod::fInPort_t>(rfMatch) =  // this will require improvement as more matches are implemented
                    [this,&nPort, &ethernet](nPort_t nPort_) {
                      nPort = nPort_;
                      Bridge::MacStatus status = m_bridge.Update( nPort_, ethernet.GetSrcMac() );
                      // need notification of src change so can update flow tables
                      
                      struct update_flow_mac_dest_actions {
                        codec::ofp_flow_mod::ofp_instruction_actions_ actions;
                        codec::ofp_flow_mod::ofp_action_output_ action;
                        void init( uint32_t nPort ) {
                          actions.init();
                          action.init( nPort );
                        }
                      };
                      
                      struct update_flow_mac_dest {
                        // once functional, could be used as a temlate for what to dow with variable matches, actions
                        codec::ofp_flow_mod::ofp_flow_mod_ mod;
                        
                        void init( const mac_t& macDest, uint32_t nPortDest) {
                          
                          mod.init();
                          
                          mod.cookie = 1000;
                          mod.idle_timeout = 10; // seconds
                          mod.priority = 100;
                          
                          auto* pMatchEthDest = new ( mod.match.oxm_fields ) codec::ofp_flow_mod::ofpxmt_ofb_eth_;
                          pMatchEthDest->init( ofp141::oxm_ofb_match_fields::OFPXMT_OFB_ETH_DST, macDest );
                          mod.match.length += sizeof( codec::ofp_flow_mod::ofpxmt_ofb_eth_ );
                          
                          mod.header.length += mod.match.length; // fixed after all oxm fields processed
                          
                          uint8_t fill_size( 0 );
                          uint8_t* pAligned = mod.fill( fill_size );
                          mod.header.length += fill_size; // skip over additional padding
                          
                          auto* pActions = new( pAligned ) update_flow_mac_dest_actions;
                          pActions->init( nPortDest );
                          pActions->actions.len += sizeof( codec::ofp_flow_mod::ofp_action_output_ );
                         
                          mod.header.length += pActions->actions.len;

                        }
                      };
                      
                      switch ( status ) {
                        case Bridge::MacStatus::Moved:
                          // remove/overwrite/add flow: match dest mac, set dest port, use expiry of 10 seconds ( for testing )
                          //break; // skip to regular action
                        case Bridge::MacStatus::Learned: {
                          // add flow: match dest mac, set dest port, use expiry of 10 seconds ( for testing )
                          vByte_t v = std::move( GetAvailableBuffer() );
                          v.resize( 
                              sizeof( codec::ofp_flow_mod::ofp_flow_mod_ ) 
                            + sizeof( codec::ofp_flow_mod::ofpxmt_ofb_eth_ )
                            + sizeof( update_flow_mac_dest_actions )
                            + 8 // maximum padding
                          );
                          auto pMod = new( v.data() ) update_flow_mac_dest; 
                          pMod->init( ethernet.GetSrcMac(), nPort );
                          //std::cout << "MOD1: " << HexDump<vByte_t::iterator>( v.begin(), v.end(), ':' ) << std::endl;
                          v.resize( pMod->mod.header.length ); // remove pading (invalidates pMod )
                          //std::cout << "MOD2: " << HexDump<vByte_t::iterator>( v.begin(), v.end(), ':' ) << std::endl;
                          //pMod->mod.command = ofp141::ofp_flow_mod_command::OFPFC_ADD;
                          //std::cout << "MissFlow: ";
                          //HexDump( std::cout, v.begin(), v.end() );
                          QueueTxToWrite( std::move( v ) );
                          }
                          break;
                        case Bridge::MacStatus::StatusQuo:
                          // issues if we reach this?
                          break;
                        case Bridge::MacStatus::Broadcast:
                          // probably just ignore this, as we already flood stuff.
                          // but maybe need to be testing for this differently
                          // probably this is an error if a broadcast is from a source.
                          break;
                      }
                      
                    };
                    
                  pMatch->decode( rfMatch ); // process match fields
                  vByte_t v = std::move( GetAvailableBuffer() );
                  codec::ofp_packet_out out;
                  out.build( v, nPort, pPacket->total_len, pPayload );
                  QueueTxToWrite( std::move( v ) );
                  
                  // expand on this to enable routing
                  switch ( ethernet.GetEthertype() ) {
                    case ethernet::Ethertype::arp: {
                      protocol::arp::Packet arp( ethernet.GetMessage() );
                      std::cout << arp << ::std::endl;
                      // maybe start a thread for other aux packet processing from above
                      }
                      break;
                  }
                  break;
                  }
                case ofp141::ofp_type::OFPT_ERROR: { // v1.4.1 page 148
                  const auto pError = new(pPacketIn) ofp141::ofp_error_msg;
                  std::cout 
                    << "Error type " << pError->type 
                    << " code " << pError->code 
                    << std::endl;
                  break;
                  }
                case ofp141::ofp_type::OFPT_FEATURES_REPLY: {
                  const auto pReply = new(pPacketIn) ofp141::ofp_switch_features;
                  codec::ofp_switch_features features( *pReply );

                  // 1.4.1 page 138
                  vByte_t v = std::move( GetAvailableBuffer() );
                  v.resize( sizeof( codec::ofp_header::ofp_header_ ) );
                  auto* p = new( v.data() ) codec::ofp_header::ofp_header_;
                  p->init();
                  p->type = ofp141::ofp_type::OFPT_GET_ASYNC_REQUEST;
                  codec::ofp_header::NewXid( *p );
                  QueueTxToWrite( std::move( v ) );

                  }
                  break;
                case ofp141::ofp_type::OFPT_ECHO_REQUEST: {
                  const auto pEcho = new(pPacketIn) ofp141::ofp_header;
                  vByte_t v = std::move( GetAvailableBuffer() );
                  v.resize( sizeof( codec::ofp_header::ofp_header_ ) );
                  auto* p = new( v.data() ) codec::ofp_header::ofp_header_;
                  p->init();
                  p->type = ofp141::ofp_type::OFPT_ECHO_REPLY;
                  p->xid = pEcho->xid;
                  if ( pEcho->length != p->length ) {
                    std::cout << "Echo request len=" << pEcho->length << " reply len=" << p->length << std::endl;
                  }
                  QueueTxToWrite( std::move( v ) );
                  }
                  break;
                case ofp141::ofp_type::OFPT_GET_ASYNC_REPLY: {
                  const auto pAsyncReply = new(pPacketIn) ofp141::ofp_async_config;
                  codec::ofp_async_config config( *pAsyncReply );
                  }
                  break;
                case ofp141::ofp_type::OFPT_PORT_STATUS: {
                  // multiple messages stacked, so need to change code in this whole section 
                  //   to sequentially parse the inbound bytes
                  const auto pStatus = new(pPacketIn) ofp141::ofp_port_status;
                  codec::ofp_port_status status( *pStatus );
                  }
                  break;
                default:
                  std::cout << "do_read unprocessed packet type: " << (uint16_t)pHeader->type << std::endl;
                  break;
              }
            }
            else {
              std::cout << "do_read no match on version: " << (uint16_t)pHeader->version << std::endl;
            }
            //do_read();  // keep the socket open with another read
            
            
            pPacketIn += pHeader->length;
          }  // end while     
          std::cout << "<<< end." << std::endl;
        } // end if ( ec )
        else {
            //std::cout << "read error: " << ec.message() << std::endl;
            // do we do another read or let it close implicitly or close explicitly?
        } // end else ( ec )
        //std::cout << "async_read end: " << std::endl;
        do_read();
      }); // end lambda
  //std::cout << "do_read end: " << std::endl;
}

//void tcp_session::do_write(std::size_t length) {
//  auto self(shared_from_this());
//  boost::asio::async_write(
//    m_socket, boost::asio::buffer(m_vTx),
//      [this, self](boost::system::error_code ec, std::size_t /*length*/)
//      {
        //if (!ec) {
        //  do_read();
        //}
//      });
//}

// TODO:  need a write queue, need to update the xid value in the header
//    so, create a method or class for handling queued messages and transactions
//void tcp_session::do_write( vChar_t& v ) {
//  auto self(shared_from_this());
//  boost::asio::async_write(
//    m_socket, boost::asio::buffer( v ),
 //     [this, self](boost::system::error_code ec, std::size_t len )
 //     {
//        std::cout << "do_write complete:" << ec << "," << len << std::endl;
        //if (!ec) {
        //  do_read();
        //}
//      });
//}

void tcp_session::do_write() {
  auto self(shared_from_this());
  //std::cout << "do_write start: " << std::endl;
  boost::asio::async_write(
    m_socket, boost::asio::buffer( m_vTxInWrite ),
      [this, self](boost::system::error_code ec, std::size_t len )
      {
        UnloadTxInWrite();
//        std::cout << "do_write atomic: " << 
        if ( 2 <= m_transmitting.fetch_sub( 1, std::memory_order_release ) ) {
          //std::cout << "do_write with atomic at " << m_transmitting.load( std::memory_order_acquire ) << std::endl;
          LoadTxInWrite();
          do_write();
        }
        //std::cout << "do_write complete:" << ec << "," << len << std::endl;
        //if (!ec) {
        //  do_read();
        //}
      });
}

void tcp_session::GetAvailableBuffer( vByte_t& v ) {
  std::unique_lock<std::mutex> lock( m_mutex );
  if ( m_qBuffersAvailable.empty() ) {
    m_qBuffersAvailable.push( vByte_t() );
  }
  v = std::move( m_qBuffersAvailable.front() );
  m_qBuffersAvailable.pop();
}

vByte_t tcp_session::GetAvailableBuffer() {
  std::unique_lock<std::mutex> lock( m_mutex );
  if ( m_qBuffersAvailable.empty() ) {
    m_qBuffersAvailable.push( vByte_t() );
  }
  vByte_t v = std::move( m_qBuffersAvailable.front() );
  m_qBuffersAvailable.pop();
  return v;
}

void tcp_session::QueueTxToWrite( vByte_t v ) {
  std::unique_lock<std::mutex> lock( m_mutex );
  //std::cout << "QTTW: " << m_transmitting.load( std::memory_order_acquire ) << std::endl;
  if ( 0 == m_transmitting.fetch_add( 1, std::memory_order_acquire ) ) {
    //std::cout << "QTTW1: " << std::endl;
    m_vTxInWrite = std::move( v );
    do_write();
  }
  else {
    //std::cout << "QTTW2: " << std::endl;
    m_qTxBuffersToBeWritten.push( std::move( v ) );
    //m_qTxBuffersToBeProcessed.push( std::move( v ) );
    //m_qTxBuffersToBeWritten.back() = std::move( v );
  }
}

void tcp_session::LoadTxInWrite() {
  std::unique_lock<std::mutex> lock( m_mutex );
  //std::cout << "LoadTxInWrite: " << std::endl;
  //assert( 0 < m_transmitting.load( std::memory_order_acquire ) );
  m_vTxInWrite = std::move( m_qTxBuffersToBeWritten.front() );
  m_qTxBuffersToBeWritten.pop();
}

void tcp_session::UnloadTxInWrite() {
  std::unique_lock<std::mutex> lock( m_mutex );
  //std::cout << "UnloadTxInWrite: " << std::endl;
  m_vTxInWrite.clear();
  m_qBuffersAvailable.push( std::move( m_vTxInWrite ) );

}
