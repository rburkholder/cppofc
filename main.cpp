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
#include <memory>

#include <boost/asio/signal_set.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/strand.hpp>

#include <boost/thread/thread.hpp>

#include <boost/log/trivial.hpp>

#include <zmq.hpp>
#include <zmq_addon.hpp>

#include "../quadlii/lib/common/monitor_t.h"
#include "../quadlii/lib/common/ZmqMessage.h"

#include "bridge.h"
#include "tcp_session.h"
#include "ovsdb.h"

namespace asio = boost::asio;
namespace ip = asio::ip;

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

  typedef asio::executor_work_guard<asio::io_context::executor_type> io_context_work;
  typedef std::shared_ptr<zmq::multipart_t> pMultipart_t;

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
    io_context_work io_work( asio::make_work_guard( io_context ) );

    boost::thread_group threads;
    for ( std::size_t ix = 0; ix < 3; ix++ ) { // TODO: how many threads required?
      threads.create_thread( boost::bind( &asio::io_context::run, &io_context ) ); // add handlers
    }

    //asio::io_context::strand strand_ovs( io_context ); // strand for transmitting ovs messages
    asio::io_context::strand strand_zmq_request( io_context );  // strand for cppof->local messages

    // https://www.boost.org/doc/libs/1_68_0/doc/html/boost_asio/overview/signals.html
    boost::asio::signal_set signals( io_context, SIGINT, SIGTERM );
    signals.async_wait( [&io_work]( const boost::system::error_code& error, int signal_number ){
      if ( !error ) {
        BOOST_LOG_TRIVIAL(trace) << "signal " << signal_number << " received.";
        // TODO: need to work on close-down process here
        io_work.reset();
        // plus other stuff (closing the sockets, for instance)
      }
    } );

    zmq::context_t zmqContext;
    zmq::socket_t zmqSocketRequest( zmqContext, zmq::socket_type::req );  // TODO construct this in which strand?
    
    asio::post( strand_zmq_request, [&zmqSocketRequest](){
      BOOST_LOG_TRIVIAL(trace) << "**** zmqSocketRequest opening ...";
      zmqSocketRequest.connect( "tcp://127.0.0.1:7411" );
      BOOST_LOG_TRIVIAL(trace) << "**** zmqSocketRequest opened.";
    } );

    // instantiate the bridge here so that it may obtain the ovsdb startup information
    //   then pass the bridge, or function calls, into the server
    // TODO: need a map of bridges, to be build from the ovs messages.
    //    start up tcp_session as the ovs messages come, one tcp_session for each bridge
    //    set for fail secure, and push out the configuration (at some point)
    //    for now, only the ones which have fail=secure set
    Bridge m_bridge;

    // open stream to ovs database for port info
    ovsdb::f_t f;

    f.fSwitchAdd = [&strand_zmq_request, &zmqSocketRequest](const ovsdb::uuid_t& uuid){
      // ovs -> local (via request):
      pMultipart_t pMultipart( new zmq::multipart_t );  // TODO: use a pool?

      msg::header hdrSnd( 1, msg::type::eOvsSwitchAdd );
      pMultipart->addtyp<msg::header>( hdrSnd );

      pMultipart->addstr( uuid );

      asio::post( strand_zmq_request, [pMultipart, &zmqSocketRequest](){

        pMultipart->send( zmqSocketRequest );

        BOOST_LOG_TRIVIAL(trace) << "**** zmqSocketRequest pmultipart is " << pMultipart->empty();

        pMultipart->recv( zmqSocketRequest );
        zmq::message_t msg;
        msg = pMultipart->pop();
        msg::header& hdrRcv( *msg.data<msg::header>() );
        BOOST_LOG_TRIVIAL(trace) << "**** zmqSocketRequest resp1: " << hdrRcv.idVersion << "," << hdrRcv.idMessage;

        msg = pMultipart->pop();
        msg::ack& msgAck( *msg.data<msg::ack>() );
        BOOST_LOG_TRIVIAL(trace) << "**** zmqSocketRequest resp2: " << msgAck.idCode;

      } );
    };

    ovsdb ovsdb_(
      io_context, f
    ); 

    server s( m_bridge, io_context, port );

    // TODO:  may need to add threads and strands 
    io_context.run();
    
    zmqSocketRequest.close();
    threads.join_all();

  }
  catch (std::exception& e)   {
    std::cerr << "Exception (at main): " << e.what() << "\n";
  }

  return 0;
}
