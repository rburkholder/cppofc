/* 
 * File:   main.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net *
 * Created on May 12, 2017, 9:15 PM
 */

/*
 Sample setup
   ovs-vsctl add-br ovsbr0
   ovs-vsctl set-fail-mode ovsbr0 secure
   #ovs-vsctl set-controller ovsbr0 tcp:0.0.0.0:6633
 * ovs-vsctl set-controller ovsbr0 tcp:127.0.0.1:6633 # keep local
   ip link set dev ovsbr0 up
   ip link set dev ovs-system up
 * 
 * other commands:
    ovs-vsctl set-fail-mode ovsbr0 secure
    ovs-vsctl del-fail-mode ovsbr0
    ovs-vsctl get-fail-mode ovsbr0
 */

#include <iostream>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "bridge.h"
#include "tcp_session.h"
#include "ovsdb.h"

namespace asio = boost::asio;
namespace ip = boost::asio::ip;

class server {
public:
  server( Bridge& bridge, asio::io_context& io_context, short port )
    : m_bridge( bridge ),
      m_acceptor( io_context, ip::tcp::endpoint( ip::tcp::v4(), port ) ),
      m_socket( io_context )
  {
    do_accept();
  }

private:

  Bridge& m_bridge;
  ip::tcp::acceptor m_acceptor;
  ip::tcp::socket m_socket;

  void do_accept() {
    m_acceptor.async_accept(
      m_socket,
      [this](boost::system::error_code ec) {
        if (!ec) {
          std::make_shared<tcp_session>(m_bridge, std::move(m_socket))->start();
        }

        // once one port started, start another acceptance
        // no recursion here as this is in a currently open session
        //   and making allowance for another session
      do_accept();
      });
  }

};

int main(int argc, char* argv[]) {

  int port( 6633 );

  try   {
    if (argc != 2) {
      std::cerr << "Usage: async_tcp_echo_server <port> (using " << port << ")\n";
//      return 1;
    }
    else {
      port = std::atoi( argv[1] );
    }

    asio::io_context io_context;
    
    // instantiate the bridge here so that it may obtain the ovsdb startup information
    //   then pass the bridge, or function calls, into the server
    Bridge m_bridge;

    // open stream to ovs database for port info
    ovsdb ovsdb_( 
      io_context,
      [](const ovsdb::switch_t&){},
      [](const ovsdb::mapPort_t&){},
      [](const ovsdb::mapInterface_t&){},
      [](const ovsdb::mapInterface_t&){}
    ); 

    // TODO:  may need to add threads and strands 
    server s( m_bridge, io_context, port );

    io_context.run();

  }
  catch (std::exception& e)   {
    std::cerr << "Exception (at main): " << e.what() << "\n";
  }

  return 0;
}
