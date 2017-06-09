/* 
 * File:   tcp_session.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 * 
 * Created on June 6, 2017, 11:07 AM
 */

#include <iostream>
#include <iomanip>

#include "codecs/common.h"
#include "codecs/ofp_header.h"
#include "codecs/ofp_hello.h"
#include "codecs/ofp_switch_features.h"
#include "codecs/ofp_async_config.h"
#include "codecs/ofp_port_status.h"
#include "codecs/ofp_flow_mod.h"

#include "tcp_session.h"

void tcp_session::start() {
  std::cout << "start begin: " << std::endl;
  do_read();
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
          static const char hex[] = "01234567890abcdef";
          std::cout << "read: " << length << "='";
          //do_write(length);
          std::cout << std::showbase << std::internal << std::setfill('0');
          for ( int ix = 0; ix < length; ++ix ) {
            uint8_t c = m_vRx[ix];
            char upper = hex[ c >> 4 ];
            char lower = hex[ c & 0x0f ];
              //std::cout << std::hex << std::setw(2) << (uint16_t)data_[ix] << " ";
            std::cout << upper << lower << " ";
          }
          std::cout << "'" <<std::dec << std::endl;
          auto pHeader = new(m_vRx.data()) ofp141::ofp_header;
          std::cout << (uint16_t)pHeader->version << "," << (uint16_t)pHeader->type << "," << pHeader->length << "," << pHeader->xid << std::endl;
          if ( OFP_VERSION == pHeader->version ) {
            switch (pHeader->type) {
              case ofp141::ofp_type::OFPT_HELLO: {
                // need to wrap following in try/catch
                const auto pHello = new(m_vRx.data()) ofp141::ofp_hello;
                codec::ofp_hello hello( *pHello );
                // do some processing
                //  then send hello back
                QueueTxToWrite( std::move( codec::ofp_hello::Create( std::move( GetAvailableBuffer() ) ) ) );
                QueueTxToWrite( std::move( codec::ofp_switch_features::CreateRequest( std::move( GetAvailableBuffer() ) ) ) );
                
                struct add_table_miss_flow {
                  codec::ofp_flow_mod::ofp_flow_mod_ mod;
                  codec::ofp_flow_mod::ofp_match_ match;
                  codec::ofp_flow_mod::ofp_instruction_actions_ actions;
                  codec::ofp_flow_mod::ofp_action_output_ action;
                  void init() {
                    mod.init();
                    match.init();
                    actions.init();
                    action.init();
                    mod.header.length = sizeof( add_table_miss_flow );
                    actions.len += sizeof( action );
                  }
                };
                vChar_t v;
                v.resize( sizeof( add_table_miss_flow ) );
                auto pMod = new( v.data() ) add_table_miss_flow; 
                pMod->init();
                pMod->mod.command = ofp141::ofp_flow_mod_command::OFPFC_ADD;
                QueueTxToWrite( std::move( v ) );
                }
                break;
              case ofp141::ofp_type::OFPT_FEATURES_REPLY: {
                const auto pReply = new(m_vRx.data()) ofp141::ofp_switch_features;
                codec::ofp_switch_features features( *pReply );

                // 1.4.1 page 138
                vChar_t v;
                v.resize( sizeof( codec::ofp_header::ofp_header_ ) );
                auto* p = new( v.data() ) codec::ofp_header::ofp_header_;
                p->init();
                p->type = ofp141::ofp_type::OFPT_GET_ASYNC_REQUEST;
                codec::ofp_header::NewXid( *p );
                QueueTxToWrite( std::move( v ) );
                
                }
                break;
              case ofp141::ofp_type::OFPT_ECHO_REQUEST: {
                const auto pEcho = new(m_vRx.data()) ofp141::ofp_header;
                vChar_t v;
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
                const auto pAsyncReply = new(m_vRx.data()) ofp141::ofp_async_config;
                codec::ofp_async_config config( *pAsyncReply );
                }
                break;
              case ofp141::ofp_type::OFPT_PORT_STATUS: {
                // multiple messages stacked, so need to change code in this whole section 
                //   to sequentially parse the inbound bytes
                const auto pStatus = new(m_vRx.data()) ofp141::ofp_port_status;
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
        }
        else {
            //std::cout << "read error: " << ec.message() << std::endl;
            // do we do another read or let it close implicitly or close explicitly?
        }
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

void tcp_session::GetAvailableBuffer( vChar_t& v ) {
  std::unique_lock<std::mutex> lock( m_mutex );
  if ( m_qBuffersAvailable.empty() ) {
    m_qBuffersAvailable.push( vChar_t() );
  }
  v = std::move( m_qBuffersAvailable.front() );
  m_qBuffersAvailable.pop();
}

vChar_t tcp_session::GetAvailableBuffer() {
  std::unique_lock<std::mutex> lock( m_mutex );
  if ( m_qBuffersAvailable.empty() ) {
    m_qBuffersAvailable.push( vChar_t() );
  }
  vChar_t v = std::move( m_qBuffersAvailable.front() );
  m_qBuffersAvailable.pop();
  return v;
}

void tcp_session::QueueTxToWrite( vChar_t v ) {
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
