/* 
 * File:   tcp_session.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 * 
 * Created on June 6, 2017, 11:07 AM
 */

// 

#include <ostream>
#include <iomanip>
#include <cstring>

#include <boost/asio/write.hpp>

#include "codecs/ofp_header.h"
#include "codecs/ofp_hello.h"
#include "codecs/ofp_switch_features.h"
#include "codecs/ofp_async_config.h"
#include "codecs/ofp_port_status.h"
#include "codecs/ofp_flow_mod.h"
#include "codecs/ofp_packet_out.h"

#include "protocol/ethernet.h"
#include "protocol/arp.h"
#include "protocol/vlan.h"

#include "common.h"
#include "hexdump.h"
#include "tcp_session.h"

namespace asio = boost::asio;

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
      [this, self](boost::system::error_code ec, const std::size_t lenRead)
      {
        //std::cout << "async_read begin: " << std::endl;
        if (!ec) {
          std::cout << ">>> total read length: " << lenRead << std::endl;

          // inbound data arrives
          // if reassembly data available;
          //    append to reassembly buffer, 
          //    clear inbound buffer
          //    move adjusted reassembly buffer to inbound buffer
          // loop on input buffer:
          //   if less then header size:
          //     move remaining octets to reassembly buffer
          //     exit loop for more
          //   if less than header.length:
          //     move remaining octets to reassembly buffer
          //     exit loop for more
          //   if >= header.length: 
          //     process packet
          //     move beginning to end
          //     loop for more
          
          std::size_t length = lenRead;
          
          if ( 0 != m_vReassembly.size() ) {
            m_vReassembly.insert( m_vReassembly.end(), m_vRx.begin(), m_vRx.begin() + lenRead );
            m_vRx.clear();
            m_vRx = std::move( m_vReassembly );
            length = m_vRx.size();
          }
          
          vByte_iter_t iterBegin = m_vRx.begin();
          vByte_iter_t iterEnd = iterBegin + length;
          
          ofp141::ofp_header* pOfpHeader;
          bool bReassemble( false );
          
          bool bLooping( true );
          
          while ( bLooping ) {
            
            auto nOctetsRemaining = iterEnd - iterBegin;

            if ( nOctetsRemaining < sizeof( ofp141::ofp_header ) ) bReassemble = true;
            else {
              pOfpHeader = new( &(*iterBegin) ) ofp141::ofp_header;
              if ( nOctetsRemaining < pOfpHeader->length ) bReassemble = true;
            }

            if ( bReassemble ) {
              // wait for more octets
              m_vReassembly = std::move( m_vRx );
              bLooping = false;
            }
            else {
              // normal packet processing
              uint8_t* pBegin = &(*iterBegin);
              ProcessPacket( pBegin, pBegin + pOfpHeader->length );
              iterBegin += pOfpHeader->length;
              bLooping = iterBegin != iterEnd;
           }
          }
          
          std::cout << "<<< end." << std::endl;

        } // end if ( ec )
        else {
          if ( 2 == ec.value() ) {
            assert( 0 );  // 'End of file', so need to re-open, or abort
          }
          else {
            std::cout << "read error: " << ec.value() << "," << ec.message() << std::endl;
          }
            // do we do another read or let it close implicitly or close explicitly?
        } // end else ( ec )
        //std::cout << "async_read end: " << std::endl;
        do_read();
      }); // end lambda
  //std::cout << "do_read end: " << std::endl;
}

void tcp_session::ProcessPacket( uint8_t* pBegin, const uint8_t* pEnd ) {
  
  ofp141::ofp_header* pHeader = new( pBegin ) ofp141::ofp_header;

  // can probably remove this check after code is validated
  auto diff = pEnd - pBegin;
  assert( 0 < diff );
  if ( pHeader->length <= diff ) {}
  else {
    auto len = pHeader->length;
    std::cout << "problem (expected,supplied): " << len << "," << diff << std::endl;
    assert( 0 );
  }

  std::cout 
    << "IN: "
    << HexDump<const uint8_t*>( pBegin, pEnd )
    << std::endl;

  std::cout 
    << (uint16_t)pHeader->version 
    << "," << (uint16_t)pHeader->type 
    << "," << pHeader->length 
    << "," << pHeader->xid 
    << std::endl;
  
  if ( OFP_VERSION == pHeader->version ) {
    switch (pHeader->type) {
      case ofp141::ofp_type::OFPT_HELLO: {
        // need to wrap following in try/catch
        const auto pHello = new(pBegin) ofp141::ofp_hello;
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
            mod.cookie = 0x101; // can change this as cookie usage becomes refined
            actions.len += sizeof( action );

            // need to update pMatch length once match fields are added                      
          }
        };
        
        vByte_t v = std::move( GetAvailableBuffer() );
        v.resize( sizeof( add_table_miss_flow ) );
        auto pMod = new( v.data() ) add_table_miss_flow; 
        pMod->init();
        pMod->mod.command = ofp141::ofp_flow_mod_command::OFPFC_ADD; // by default
        std::cout 
          << "Sent MissFlow flow entry: " 
          << HexDump<vByte_iter_t>( v.begin(), v.end() )
          << std::endl;
        QueueTxToWrite( std::move( v ) );
        
        // TODO:  install two flows (higher priority than default packet_in):
        //   match src broadcast -> drop (should there be such an animal?)
        //   match dst broadcast -> flood

        }
        break;
      case ofp141::ofp_type::OFPT_PACKET_IN: { // v1.4.1 page 140

        // rather than flood (output), re-use the table when possible
        // now should be able to modularize this code

        const auto pPacket = new(pBegin) ofp141::ofp_packet_in;
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
        const auto pPayload = pBegin + pPacket->header.length - pPacket->total_len;
        std::cout 
          << "  content: "
          << HexDump<uint8_t*>( pPayload, pPayload + pPacket->total_len )
          << ::std::endl;

        ethernet::header ethernet( *pPayload ); // pull out ethernet header
        std::cout << ethernet << ::std::endl;

        // expand on this to enable routing, for now, is just random information
        switch ( ethernet.GetEthertype() ) {
          case ethernet::Ethertype::arp: {
            protocol::arp::Packet arp( ethernet.GetMessage() );
            std::cout << arp << ::std::endl;
            // maybe start a thread for other aux packet processing from above
            }
            break;
          case ethernet::Ethertype::ieee8021q: {  // vlan
            ethernet::vlan vlan( ethernet.GetMessage() );
            std::cout << vlan << ::std::endl;
            }
            break;
          case ethernet::Ethertype::ieee8021ad: // QinQ
            break;
          case ethernet::Ethertype::ipv4:
            break;
          case ethernet::Ethertype::ipv6:
            break;
        }
        
        // create a lambda (TODO: needs to be restructured, for handling message
        //   needs to be integral to the cookie switch statement (to be refactored)
        uint32_t nSrcPort;
        codec::ofp_flow_mod::fCookie0x101_t fCookie0x101 =  // this will require improvement as more matches are implemented
          [this, &nSrcPort, &ethernet](nPort_t nSrcPort_) -> codec::ofp_flow_mod::Verdict {

            struct update_flow_mac_dest_actions {
              codec::ofp_flow_mod::ofp_instruction_actions_ actions;
              codec::ofp_flow_mod::ofp_action_output_ action;
              void init( uint32_t nPort ) {
                actions.init();
                action.init( nPort );
              }
            };

            struct update_flow_mac_src_dest {
              // once functional, could be used as a template for what to do with variable number of matches, actions
              codec::ofp_flow_mod::ofp_flow_mod_ mod;

              void init( const mac_t& macSrc, const mac_t& macDst, uint32_t nPortDest ) {

                mod.init();

                mod.cookie = 0x201;
                mod.idle_timeout = 10; // seconds
                mod.priority = 100;
                
                // pMatch used as placeholder for match structures
                boost::endian::big_uint8_t* pMatch = &mod.match.oxm_fields[0];

                // create match on source mac
                auto* pMatchEthSrc = new ( pMatch ) codec::ofp_flow_mod::ofpxmt_ofb_eth_;
                pMatchEthSrc->init( ofp141::oxm_ofb_match_fields::OFPXMT_OFB_ETH_SRC, macSrc );
                mod.match.length += sizeof( codec::ofp_flow_mod::ofpxmt_ofb_eth_ );
                
                // match structures start from here
                pMatch += sizeof( codec::ofp_flow_mod::ofpxmt_ofb_eth_ );

                // create match on destination mac
                auto* pMatchEthDst = new ( pMatch ) codec::ofp_flow_mod::ofpxmt_ofb_eth_;
                pMatchEthDst->init( ofp141::oxm_ofb_match_fields::OFPXMT_OFB_ETH_DST, macDst );
                mod.match.length += sizeof( codec::ofp_flow_mod::ofpxmt_ofb_eth_ );

                // update overall length from match structures
                mod.header.length += mod.match.length; // fixed after all oxm fields processed

                // standard requires some padding
                uint8_t fill_size( 0 );
                uint8_t* pAligned = mod.fill( fill_size );
                mod.header.length += fill_size; // skip over additional padding

                // set action for output port
                auto* pActions = new( pAligned ) update_flow_mac_dest_actions;
                pActions->init( nPortDest );
                pActions->actions.len += sizeof( codec::ofp_flow_mod::ofp_action_output_ );

                // update overall length from action structures
                mod.header.length += pActions->actions.len;

              } // init()
            }; // update_flow_mac_src_dest
            
            MacAddress macSrc( ethernet.GetSrcMac() );
            MacAddress macDst( ethernet.GetDstMac() );

            nSrcPort = nSrcPort_;
            Bridge::MacStatus statusSrcLookup = m_bridge.Update( nSrcPort_, macSrc.Value() );
            
            nPort_t nDstPort = m_bridge.Lookup( ethernet.GetDstMac() );;
            
            // if we arrived in this code, it means a flow or set of flows:
            //   a) have not existed, or
            //   b) have expired
            
            typedef codec::ofp_flow_mod::Verdict Verdict;
            Verdict verdict( Verdict::Drop );
            
            if ( MacAddress::IsMulticast( macSrc ) 
              || MacAddress::IsBroadcast( macSrc )
            ) {
              std::cout << "source based broadcast/multicast address found" << std::endl;
            }
            else {
              if (   MacAddress::IsMulticast( macDst ) 
                ||   MacAddress::IsBroadcast( macDst )
                || ( ofp141::ofp_port_no::OFPP_MAX <= nDstPort )
              ) {
                // need to flood the packet
                verdict = Verdict::Flood;
              }
              else {
                // configure flow in each direction
                // remove/overwrite/add flow: match dest mac, set dest port,
                // if a mac-dest only flow is inserted, 
                //   won't get a packet_in from the other direction to learn those macs
                // therefore, only insert src/dest based flows, both ways
                //   use flow expiry of 10 seconds for testing
                //   match srcmac, destmac, set dest port
                //   put in both directions
                
                {
                  vByte_t v = std::move( GetAvailableBuffer() );
                  v.resize( max_length );
                  auto pMod = new( v.data() ) update_flow_mac_src_dest; 
                  pMod->init( ethernet.GetSrcMac(), ethernet.GetDstMac(), nDstPort );
                  v.resize( pMod->mod.header.length ); // remove padding (invalidates pMod )
                  //std::cout << "MOD1: " << HexDump<vByte_t::iterator>( v.begin(), v.end(), ' ' ) << std::endl;
                  QueueTxToWrite( std::move( v ) );
                  std::cout << "Update Flow1: " 
                    << std::hex << nDstPort << std::dec << std::endl;
                }

                {
                  vByte_t v = std::move( GetAvailableBuffer() );
                  v.resize( max_length );
                  auto pMod = new( v.data() ) update_flow_mac_src_dest; 
                  pMod->init( ethernet.GetDstMac(), ethernet.GetSrcMac(), nSrcPort );
                  v.resize( pMod->mod.header.length ); // remove padding (invalidates pMod )
                  //std::cout << "MOD2: " << HexDump<vByte_t::iterator>( v.begin(), v.end(), ' ' ) << std::endl;
                  QueueTxToWrite( std::move( v ) );
                  std::cout << "Update Flow2: " << nSrcPort << std::endl;
                }
                
                verdict = Verdict::Directed;
              }
            }
            return verdict;
          }; // end of lambda( in_port )

        // three choices - refactor in to switch ( status ) statement above
        // 1) flood to all ports if dest mac not found
        // 2) flood to all ports if broadcast mac found
        // 3) send to table if found in bridge table (flow should have been installed above)
        switch ( pPacket->cookie ) {
          case 0x101: {
            typedef codec::ofp_flow_mod::Verdict Verdict;
            Verdict verdict = pMatch->decode( fCookie0x101 ); // process match fields via the lambda
            switch ( verdict ) {
              case Verdict::Drop:
                // do nothing
                std::cout << "Verdict Drop " << std::endl;
                break;
              case Verdict::Directed:{
                // send via table?  // TODO: fix this to be a send via table, may need a barrier message
                vByte_t v = std::move( GetAvailableBuffer() );
                codec::ofp_packet_out out; // flood for now, but TODO: run through tables again, if not broadcast
                //out.build( v, nSrcPort, pPacket->total_len, pPayload, ofp141::ofp_port_no::OFPP_TABLE );
                out.build( v, nSrcPort, pPacket->total_len, pPayload, ofp141::ofp_port_no::OFPP_ALL );
                QueueTxToWrite( std::move( v ) );
                std::cout << "Verdict Directed " << std::endl;
                }
                
                break;
              case Verdict::Flood: {
                // flood via ALL
                vByte_t v = std::move( GetAvailableBuffer() );
                codec::ofp_packet_out out; // flood for now, but TODO: run through tables again, if not broadcast
                out.build( v, nSrcPort, pPacket->total_len, pPayload ); // set for flood
                QueueTxToWrite( std::move( v ) );
                std::cout << "Verdict Flood" << std::endl;
                }
                break;
            }
            }
            break;
          default:
            assert( 0 );  // need to catch unknown packet_in, shouldn't be any
            break;
        }

        break;
        }
      case ofp141::ofp_type::OFPT_ERROR: { // v1.4.1 page 148
        const auto pError = new(pBegin) ofp141::ofp_error_msg;
        std::cout 
          << "Error type " << pError->type 
          << " code " << pError->code 
          << std::endl;
        break;
        }
      case ofp141::ofp_type::OFPT_FEATURES_REPLY: {
        const auto pReply = new(pBegin) ofp141::ofp_switch_features;
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
        const auto pEcho = new(pBegin) ofp141::ofp_header;
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
        const auto pAsyncReply = new(pBegin) ofp141::ofp_async_config;
        codec::ofp_async_config config( *pAsyncReply );
        }
        break;
      case ofp141::ofp_type::OFPT_PORT_STATUS: {
        // multiple messages stacked, so need to change code in this whole section 
        //   to sequentially parse the inbound bytes
        const auto pStatus = new(pBegin) ofp141::ofp_port_status;
        codec::ofp_port_status status( *pStatus );
        }
        break;
      default:
        std::cout << "do_read unprocessed packet type: " << (uint16_t)pHeader->type << std::endl;
        break;
    }
  }
  else {
    //std::cout << "do_read no match on version: " << (uint16_t)pHeader->version << std::endl;
  }
  //do_read();  // keep the socket open with another read
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
  asio::async_write(
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
