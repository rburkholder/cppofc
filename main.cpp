/* 
 * File:   main.cpp
 * Author: vagrant
 *
 * Created on May 12, 2017, 9:15 PM
 */

//#include <cstdlib>
#include <iostream>
//#include <memory>
//#include <utility>
//#include <vector>

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