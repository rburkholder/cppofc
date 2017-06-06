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
#include "codecs/ofp_hello.h"
#include "codecs/ofp_switch_features.h"

#include "tcp_session.h"

void tcp_session::start() {
  std::cout << "start begin: " << std::endl;
  do_read();
  std::cout << "start end: " << std::endl;
}

void tcp_session::do_read() {
  std::cout << "do_read begin: " << std::endl;
  // going to need to perform serialization as bytes come in (multiple packets joined together)?
  auto self(shared_from_this());
  m_vRx.resize( max_length );
  m_socket.async_read_some(boost::asio::buffer(m_vRx),
      [this, self](boost::system::error_code ec, std::size_t length)
      {
        if (!ec) {
          char hex[] = "01234567890abcdef";
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
              case ofp141::OFPT_HELLO: {
                // need to wrap following in try/catch
                const auto pHello = new(m_vRx.data()) ofp141::ofp_hello;
                codec::ofp_hello hello( *pHello );
                // do some processing
                //  then send hello back
                codec::ofp_hello::Create( m_vTx );
                do_write( m_vTx );
                }
                break;
              case ofp141::OFPT_FEATURES_REPLY: {
                const auto pReply = new(m_vRx.data()) ofp141::ofp_switch_features;
                codec::ofp_switch_features features( *pReply );
                }
                break;
              default:
                std::cout << "do_read packet type: " << (uint16_t)pHeader->type << std::endl;
                break;
            }
          }
          else {
            std::cout << "do_read no match on version: " << (uint16_t)pHeader->version << std::endl;
          }
          do_read();  // keep the socket open with another read
        } // end lambda
        else {
            std::cout << "read error: " << ec.message() << std::endl;
        }
      });
  std::cout << "do_read end: " << std::endl;
}

void tcp_session::do_write(std::size_t length) {
  auto self(shared_from_this());
  boost::asio::async_write(
    m_socket, boost::asio::buffer(m_vTx),
      [this, self](boost::system::error_code ec, std::size_t /*length*/)
      {
        //if (!ec) {
        //  do_read();
        //}
      });
}

// TODO:  need a write queue, need to update the xid value in the header
//    so, create a method or class for handling queued messages and transactions
void tcp_session::do_write( vChar_t& v ) {
  auto self(shared_from_this());
  boost::asio::async_write(
    m_socket, boost::asio::buffer( v ),
      [this, self](boost::system::error_code ec, std::size_t len )
      {
        std::cout << "do_write complete:" << ec << "," << len << std::endl;
        //if (!ec) {
        //  do_read();
        //}
      });
}

