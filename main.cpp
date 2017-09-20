/* 
 * File:   main.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net *
 * Created on May 12, 2017, 9:15 PM
 */

#include <iostream>

#include <boost/asio.hpp>

#include "tcp_session.h"

using boost::asio::ip::tcp;

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
            std::make_shared<tcp_session>(std::move(socket_))->start();
          }

          // once one port started, start another acceptance
          // no recursion here as this is in a currently open session
          //   and making allowance for another session
          do_accept();
        });
  }

  tcp::acceptor acceptor_;
  tcp::socket socket_;
};

int main(int argc, char* argv[]) {
    
  int port( 6633 );
    
  boost::asio::io_service io_service;

  try   {
    if (argc != 2) {
      std::cerr << "Usage: async_tcp_echo_server <port> (using " << port << ")\n";
//      return 1;
    }
    else {
      port = std::atoi(argv[1]);
    }

    server s(io_service, port);

    io_service.run();
  }
  catch (std::exception& e)   {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}