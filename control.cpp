/* 
 * File:   control.cpp (renamed from main.cpp 2018/11/22)
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

#include <memory>

#include <boost/asio/post.hpp>

#include <boost/log/trivial.hpp>

#include "../quadlii/lib/common/monitor_t.h"
#include "../quadlii/lib/common/ZmqMessage.h"

#include "tcp_session.h"

#include "control.h"

Control::Control( int port )
:
  m_port( port ),
  m_signals( m_ioContext, SIGINT, SIGTERM ),
  m_strandZmqRequest( m_ioContext ),
  m_zmqSocketRequest( m_zmqContext, zmq::socket_type::req ),  // TODO construct this in which strand?
  //m_ovsdb( m_ioContext, m_f ),
  m_socket( m_ioContext ),
  m_acceptor( m_ioContext, ip::tcp::endpoint( ip::tcp::v4(), port ) ),
  m_ioWork( asio::make_work_guard( m_ioContext ) )
{
}

Control::~Control() {
  m_ioWork.reset();
  m_zmqSocketRequest.close();
  m_threads.join_all();
}

void Control::Start() {

  try   {

    for ( std::size_t ix = 0; ix < 3; ix++ ) { // TODO: how many threads required?
      m_threads.create_thread( boost::bind( &asio::io_context::run, &m_ioContext ) ); // add handlers
    }

    // https://www.boost.org/doc/libs/1_68_0/doc/html/boost_asio/overview/signals.html
    m_signals.async_wait( [this]( const boost::system::error_code& error, int signal_number ){
      if ( !error ) {
        BOOST_LOG_TRIVIAL(trace) << "signal " << signal_number << " received.";
        // TODO: need to work on close-down process here
        m_ioWork.reset();
        // plus other stuff (closing the sockets, for instance)
      }
    } );

    asio::post( m_strandZmqRequest, [this](){
      BOOST_LOG_TRIVIAL(trace) << "**** m_zmqSocketRequest opening ...";
      m_zmqSocketRequest.connect( "tcp://127.0.0.1:7411" );
      BOOST_LOG_TRIVIAL(trace) << "**** m_zmqSocketRequest opened.";
    } );

    // TODO: need a map of bridges, to be build from the ovs messages.
    //    start up tcp_session as the ovs messages come, one tcp_session for each bridge
    //    set for fail secure, and push out the configuration (at some point)
    //    for now, only the ones which have fail=secure set
    
    namespace ph = std::placeholders;

    m_f.fSwitchAdd = std::bind( &Control::HandleSwitchAdd, this, ph::_1 );
    ovsdb m_ovsdb( m_ioContext, m_f );

    AcceptControlConnections();

    m_ioContext.run();

  }
  catch (std::exception& e)   {
    BOOST_LOG_TRIVIAL(trace) << "Exception (at main): " << e.what() << "\n";
  }

}

void Control::AcceptControlConnections() {
  m_acceptor.async_accept(
    m_socket,
    [this](boost::system::error_code ec) {
      if (!ec) {
        std::make_shared<tcp_session>(m_bridge, std::move(m_socket))->start();
      }

      // once one port started, start another acceptance
      // no recursion here as this is in a currently open session
      //   and making allowance for another session
    AcceptControlConnections();
    });
}

void Control::PostToZmqRequest( pMultipart_t pMultipart ) {
  asio::post( m_strandZmqRequest, [this, pMultipart](){

    pMultipart->send( m_zmqSocketRequest );

    BOOST_LOG_TRIVIAL(trace) << "**** m_zmqSocketRequest pmultipart is " << pMultipart->empty();

    pMultipart->recv( m_zmqSocketRequest );
    zmq::message_t msg;
    msg = pMultipart->pop();
    msg::header& hdrRcv( *msg.data<msg::header>() );
    BOOST_LOG_TRIVIAL(trace) << "**** m_zmqSocketRequest resp1: " << hdrRcv.idVersion << "," << hdrRcv.idMessage;

    assert( msg::type::eAck == hdrRcv.id() );

    msg = pMultipart->pop();
    msg::ack& msgAck( *msg.data<msg::ack>() );
    BOOST_LOG_TRIVIAL(trace) << "**** m_zmqSocketRequest resp2: " << msgAck.idCode;

    assert( msg::ack::code::ok == msgAck.idCode );

  } );
}

void Control::HandleSwitchAdd( const ovsdb::uuid_t& uuid ) {
  // ovs -> local (via request):
  pMultipart_t pMultipart( new zmq::multipart_t );  // TODO: use a pool?

  msg::header hdrSnd( 1, msg::type::eOvsSwitchAdd );
  pMultipart->addtyp<msg::header>( hdrSnd );

  pMultipart->addstr( uuid );

  PostToZmqRequest( pMultipart );
}
