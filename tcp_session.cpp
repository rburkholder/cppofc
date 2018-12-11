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

#include <boost/log/trivial.hpp>

#include <boost/asio/write.hpp>

#include "codecs/ofp_header.h"
#include "codecs/ofp_hello.h"
#include "codecs/ofp_switch_features.h"
#include "codecs/ofp_async_config.h"
#include "codecs/ofp_port_status.h"
#include "codecs/ofp_flow_mod.h"
#include "codecs/ofp_packet_out.h"

#include "protocol/ethernet.h"
#include "protocol/ethernet/vlan.h"
#include "protocol/ipv4.h"
#include "protocol/ipv4/udp.h"
#include "protocol/ipv4/tcp.h"
#include "protocol/ipv6.h"

#include "protocol/ipv4/dhcp.h"

#include "common.h"
#include "hexdump.h"
#include "tcp_session.h"

namespace asio = boost::asio;

// 2017/09/28 - once the bridge works, then can start work on routing:
//   https://dtucker.co.uk/hack/building-a-router-with-openvswitch.html

// 2017/09/28 - test port up/down/add/delete messages

// 2018/12/08 test for more packet lengths.

  tcp_session::tcp_session( Bridge& bridge, ip::tcp::socket socket)
    : m_bridge( bridge ),
      m_socket( std::move( socket ) ),
      m_transmitting( 0 )
  {
    BOOST_LOG_TRIVIAL(trace) << "tcp_session construction";
  }

  tcp_session::~tcp_session() {
    BOOST_LOG_TRIVIAL(trace) << "tcp_session destruction";
  }

void tcp_session::start() {
  try {
    do_read();
  }
  catch(...) {
    BOOST_LOG_TRIVIAL(trace) << "tcp_session::do_read issues";
  }
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
          std::cout << "read error: " << ec.value() << "," << ec.message() << std::endl;
          if ( 2 == ec.value() ) {
            assert( 0 );  // 'End of file', so need to re-open, or abort
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

  if ( false ) {
    std::cout
      << "IN: "
      << HexDump<const uint8_t*>( pBegin, pEnd )
      << std::endl;
  }

  if ( false ) {
    std::cout
      <<  "  ver=" << (uint16_t)pHeader->version
      << ", type=" << (uint16_t)pHeader->type
      <<  ", len=" << pHeader->length
      <<  ", xid=" << pHeader->xid
      << std::endl;
  }

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

        // Start bridge to update groups and forwarding rules
        // TODO: need a strand for the bridge?  What threads use the bridge?
        //std::cout << "** tcp_session::m_bRulesInjectionActive calling StartRulesInjection" << std::endl;
        m_bridge.StartRulesInjection(
          // fAcquireBuffer
          [this]()->vByte_t{
            return std::move( GetAvailableBuffer() );
          },
          // fTransmitBuffer
          [this]( vByte_t v ){
            QueueTxToWrite( std::move( v ) );
          } );

        // this table miss entry then starts to generate Packet_in messages
        struct add_table_miss_flow {
          codec::ofp_flow_mod::ofp_flow_mod_ mod;
          codec::ofp_flow_mod::ofp_instruction_actions_ actions;
          codec::ofp_flow_mod::ofp_action_output_ action;
          void init() {
            mod.init();
            actions.init();
            action.init();
            action.max_len = ofp141::ofp_controller_max_len::OFPCML_NO_BUFFER;
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

        protocol::ethernet::header ethernet( *pPayload ); // pull out ethernet header
        std::cout << ethernet << ::std::endl;

        // expand on this to enable routing, for now, is just random information
        uint16_t idVlan( 0 ); // init to 0 if no 802.1q header found (will need to deal with QinQ at some point)
        uint8_t* pMessage( nullptr ); // depending upon 802.1q shim, calculate message location
        uint16_t idEtherType( 0 );
        if ( protocol::ethernet::Ethertype::ieee8021q == ethernet.GetEthertype() ) {
          ethernet::vlan vlan( ethernet.GetMessage() );
          std::cout << "found vlan: " << vlan << ::std::endl;
          idVlan = vlan.GetVID();
          pMessage = &vlan.GetMessage();
          idEtherType = vlan.GetEthertype();
        }
        else {
          pMessage = &ethernet.GetMessage();
          idEtherType = ethernet.GetEthertype();
        }
        switch ( idEtherType ) {
          case protocol::ethernet::Ethertype::arp: {
            protocol::ipv4::arp::IPv4Ether arp( *pMessage );
            m_arpCache.Update( arp );
            std::cout << arp << ::std::endl;
            // maybe start a thread for other aux packet processing from above
            }
            break;
          case protocol::ethernet::Ethertype::ieee8021q: {  // 802.1q vlan (shouldn't be able to get here )
            ethernet::vlan vlan( *pMessage );
            std::cout << "bad vlan in vlan found: " << vlan << ::std::endl;
            assert( 0 ); // not dealing with vlan in vlan
            }
            break;
          case protocol::ethernet::Ethertype::ipv4: {
            protocol::ipv4::Packet ipv4( *pMessage );
            std::cout << ipv4 << ::std::endl;

            switch ( ipv4.GetHeader().protocol ) {
              case 6: {// tcp
                protocol::tcp::Packet tcp( ipv4.GetData() );
                std::cout << tcp << ::std::endl;
                }
                break;
              case 17: {// udp
                protocol::udp::Packet udp( ipv4.GetData() );
                std::cout << udp << ::std::endl;
                if ( 67 == udp.GetHeader().dst_port ) {
                  protocol::ipv4::dhcp::Packet dhcp( udp.GetData() );
                  std::cout << dhcp << ::std::endl;
                }
                }
                break;
              }
            }
            break;
          case protocol::ethernet::Ethertype::ipv6: {
            protocol::ipv6::Packet ipv6( *pMessage );
            std::cout << ipv6 << ::std::endl;
            }
            break;
        }

        // create a lambda (TODO: needs to be restructured, for handling message
        //   needs to be integral to the cookie switch statement (to be refactored)
        codec::ofp_flow_mod::fCookie0x101_t fCookie0x101 =  // this will require improvement as more matches are implemented
          // nSrcPort_ comes from match decode
          [this, idVlan, &ethernet, pPayload, length = pPacket->total_len](nPort_t nSrcPort) {

            typedef protocol::ethernet::address MacAddress;

            MacAddress macSrc( ethernet.GetSrcMac() );
            MacAddress macDst( ethernet.GetDstMac() );

            Bridge::MacStatus statusSrcLookup = m_bridge.Update( nSrcPort, idVlan, macSrc );
            m_bridge.Forward( nSrcPort, idVlan, macSrc, macDst, pPayload, length );

          }; // end of lambda( in_port )

        switch ( pPacket->cookie ) {
          case 0x101: {
            // for decoding the IN_PORT to supply to the bridge
            pMatch->decode( fCookie0x101 ); // process match fields via the lambda
            }
            break;
          default:
            assert( 0 );  // need to catch unknown packet_in, shouldn't be any
            // could be because of mal-formed packet if commands are incorrect
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
        std::cout
          << "ofp141::ofp_type::OFPT_ECHO_REQUEST received/replied"
          << std::endl;
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
        std::cout
          << "ofp141::ofp_type::OFPT_GET_ASYNC_REPLY (after features_reply):"
          << std::endl;
        const auto pAsyncReply = new(pBegin) ofp141::ofp_async_config;
        codec::ofp_async_config config( *pAsyncReply );
        }
        break;
      case ofp141::ofp_type::OFPT_PORT_STATUS: {
        // multiple messages stacked, so need to change code in this whole section
        //   to sequentially parse the inbound bytes -- 2018/11/13 I think this is taken care of now
        // TODO: use this to update the gui, to confirm what other parts of the engine are saying (ovsdb code does something similar)
        const auto pStatus = new(pBegin) ofp141::ofp_port_status;
        codec::ofp_port_status status( *pStatus );
        std::cout
          << "ofp141::ofp_type::OFPT_PORT_STATUS"
          << std::endl;
        }
        break;
      case ofp141::ofp_type::OFPT_BARRIER_REPLY: {
        std::cout
          << "ofp141::ofp_type::OFPT_BARRIER_REPLY"
          << std::endl;
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
  auto self( shared_from_this() );
  //std::cout << "do_write start: " << std::endl;
  if ( 0 == m_bufferTxQueue.Front().size() ) {
    assert( 0 );
  }
  if ( false ) {
    std::cout
      << "OUT: " << std::endl
      << "00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f" << std::endl
      << HexDump<vByte_t::const_iterator>( m_bufferTxQueue.Front().begin(), m_bufferTxQueue.Front().end() )
      << std::endl;
  }

  asio::async_write(
    m_socket, boost::asio::buffer( m_bufferTxQueue.Front() ),  // rather than class variable, pass contents into lambda
      [this, self]( boost::system::error_code ec, std::size_t len )
      {
        std::unique_lock<std::mutex> lock( m_mutex );
        vByte_t v = std::move( m_bufferTxQueue.ObtainBuffer() );
        v.clear();
        m_bufferAvailable.AddBuffer( v );
//        std::cout << "do_write atomic: " <<
        if ( 2 <= m_transmitting.fetch_sub( 1, std::memory_order_release ) ) {
          //std::cout << "do_write with atomic at " << m_transmitting.load( std::memory_order_acquire ) << std::endl;
          //m_vTxInWrite = std::move( m_bufferTxQueue.ObtainBuffer() );
          do_write();
        }
        //std::cout << "do_write complete:" << ec << "," << len << std::endl;
        //if (!ec) {
        //  do_read();
        //}
      });
}

vByte_t tcp_session::GetAvailableBuffer() {
  std::unique_lock<std::mutex> lock( m_mutex );
  return m_bufferAvailable.ObtainBuffer();
}

// TODO: run these methods in a strand?
void tcp_session::QueueTxToWrite( vByte_t v ) { // TODO: look at changing to lvalue ref or rvalue ref
  std::unique_lock<std::mutex> lock( m_mutex );
  //std::cout << "QTTW: " << m_transmitting.load( std::memory_order_acquire ) << std::endl;
  if ( 0 == v.size() ) {
    assert( 0 );
  }
  m_bufferTxQueue.AddBuffer( v );
  if ( 0 == m_transmitting.fetch_add( 1, std::memory_order_acquire ) ) {
    //std::cout << "QTTW1: " << std::endl;
    do_write();
  }
}

