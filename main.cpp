/* 
 * File:   main.cpp
 * Author: vagrant
 *
 * Created on May 12, 2017, 9:15 PM
 */

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <iomanip>
#include <vector>

#include <boost/asio.hpp>

//#include <boost/endian/arithmetic.hpp>
//using namespace boost::endian;
//#include "openflow/openflow-spec1.4.1.h"

#include "codecs/common.h"
#include "codecs/ofp_hello.h"

using boost::asio::ip::tcp;

class session
  : public std::enable_shared_from_this<session>
{
public:
  session(tcp::socket socket)
    : m_socket(std::move(socket))
  {
  }

  void start() {
    do_read();
  }

private:
  void do_read() {
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
                case 0:
                  // need to wrap following in try/catch
                  const auto pHello = new(m_vRx.data()) ofp141::ofp_hello;
                  codec::ofp_hello hello( *pHello );
                  // do some processing
                  //  then send hello back
                  codec::ofp_hello::Create( m_vTx );
                  do_write( m_vTx );
                  break;
              }
            }
        
          }
          else {
              std::cout << "error: " << ec.message() << std::endl;
          }
        });
  }

  void do_write(std::size_t length) {
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

  void do_write( vChar_t& v ) {
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

  tcp::socket m_socket;
  enum { max_length = 17000 };
  //char m_pktTx[max_length];
  //char m_pktRx[max_length];
  //typedef std::vector<char> vChar_t;
  vChar_t m_vRx;
  vChar_t m_vTx;
};

class server {
public:
  server(boost::asio::io_service& io_service, short port)
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
      socket_(io_service)
  {
    do_accept();
  }

private:
  void do_accept() {
    acceptor_.async_accept(socket_,
        [this](boost::system::error_code ec)
        {
          if (!ec) {
            std::make_shared<session>(std::move(socket_))->start();
          }

          do_accept();
        });
  }

  tcp::acceptor acceptor_;
  tcp::socket socket_;
};

int main(int argc, char* argv[]) {
  try   {
    if (argc != 2) {
      std::cerr << "Usage: async_tcp_echo_server <port>\n";
      return 1;
    }

    boost::asio::io_service io_service;

    server s(io_service, std::atoi(argv[1]));
    //server s( io_service, 6653) ;

    io_service.run();
  }
  catch (std::exception& e)   {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}