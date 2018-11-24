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
      //BOOST_LOG_TRIVIAL(trace) << "**** m_zmqSocketRequest opening ...";
      m_zmqSocketRequest.connect( "tcp://127.0.0.1:7411" );
      //BOOST_LOG_TRIVIAL(trace) << "**** m_zmqSocketRequest opened.";
    } );

    // TODO: need a map of bridges, to be build from the ovs messages.
    //    start up tcp_session as the ovs messages come, one tcp_session for each bridge
    //    set for fail secure, and push out the configuration (at some point)
    //    for now, only the ones which have fail=secure set

    namespace ph = std::placeholders;

    ovsdb::f_t f;

    f.fSwitchAdd    = std::bind( &Control::HandleSwitchAdd, this, ph::_1 );
    f.fSwitchUpdate = std::bind( &Control::HandleSwitchUpdate, this, ph::_1, ph::_2 );
    f.fSwitchDelete = std::bind( &Control::HandleSwitchDelete, this, ph::_1 );

    f.fBridgeAdd    = std::bind( &Control::HandleBridgeAdd, this, ph::_1, ph::_2 );
    f.fBridgeUpdate = std::bind( &Control::HandleBridgeUpdate, this, ph::_1, ph::_2 );
    f.fBridgeDelete = std::bind( &Control::HandleBridgeDelete, this, ph::_1 );

    f.fPortAdd    = std::bind( &Control::HandlePortAdd, this, ph::_1, ph::_2 );
    f.fPortUpdate = std::bind( &Control::HandlePortUpdate, this, ph::_1, ph::_2 );
    f.fPortDelete = std::bind( &Control::HandlePortDelete, this, ph::_1 );

    f.fInterfaceAdd    = std::bind( &Control::HandleInterfaceAdd, this, ph::_1, ph::_2 );
    f.fInterfaceUpdate = std::bind( &Control::HandleInterfaceUpdate, this, ph::_1, ph::_2 );
    f.fInterfaceDelete = std::bind( &Control::HandleInterfaceDelete, this, ph::_1 );

    f.fStatisticsUpdate = std::bind( &Control::HandleStatisticsUpdate, this, ph::_1, ph::_2 );

    ovsdb m_ovsdb( m_ioContext, f );

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

// Move capture in lambda
// https://stackoverflow.com/questions/8640393/move-capture-in-lambda
void Control::PostToZmqRequest( pMultipart_t& pMultipart ) {
  try {
    asio::post( m_strandZmqRequest, [this, pMultipart_ = std::move( pMultipart)](){

      BOOST_LOG_TRIVIAL(trace) << "**** m_zmqSocketRequest sending ...";

      pMultipart_->send( m_zmqSocketRequest );

      BOOST_LOG_TRIVIAL(trace) << "**** m_zmqSocketRequest pmultipart is " << pMultipart_->empty();

      pMultipart_->recv( m_zmqSocketRequest );
      zmq::message_t msg;
      msg = pMultipart_->pop();
      msg::header& hdrRcv( *msg.data<msg::header>() );
      BOOST_LOG_TRIVIAL(trace) << "**** m_zmqSocketRequest resp1: " << hdrRcv.idVersion << "," << hdrRcv.idMessage;

      assert( msg::type::eAck == hdrRcv.id() );

      msg = pMultipart_->pop();
      msg::ack& msgAck( *msg.data<msg::ack>() );
      BOOST_LOG_TRIVIAL(trace) << "**** m_zmqSocketRequest resp2: " << msgAck.idCode;

      assert( msg::ack::code::ok == msgAck.idCode );

    } );
  }
  catch (...) {
    BOOST_LOG_TRIVIAL(trace) << "Control::PostToZmqRequest problems";
  }
}

void Control::HandleSwitchAdd( const ovsdb::uuidSwitch_t& uuidSwitch ) {
  // ovs -> local (via request):
  auto pMultipart = std::make_unique<zmq::multipart_t>();  // TODO: use a pool?

  msg::header hdrSnd( 1, msg::type::eOvsSwitchAdd );
  pMultipart->addtyp<msg::header>( hdrSnd );

  pMultipart->addstr( uuidSwitch );

  PostToZmqRequest( pMultipart );
}

void Control::HandleSwitchUpdate( const ovsdb::uuidSwitch_t& uuidSwitch, const ovsdb::switch_t& sw ) {
  // ovs -> local (via request):
  auto pMultipart = std::make_unique<zmq::multipart_t>();  // TODO: use a pool?

  msg::header hdrSnd( 1, msg::type::eOvsSwitchUpdate );
  pMultipart->addtyp<msg::header>( hdrSnd );

  pMultipart->addstr( uuidSwitch );
  pMultipart->addstr( sw.hostname );
  pMultipart->addstr( sw.ovs_version );
  pMultipart->addstr( sw.db_version );

  PostToZmqRequest( pMultipart );
}

void Control::HandleSwitchDelete( const ovsdb::uuidSwitch_t& uuidSwitch ) {
}

void Control::HandleBridgeAdd( const ovsdb::uuidSwitch_t& uuidSwitch, const ovsdb::uuid_t& uuidBridge ) {
  // ovs -> local (via request):
  auto pMultipart = std::make_unique<zmq::multipart_t>();  // TODO: use a pool?

  msg::header hdrSnd( 1, msg::type::eOvsBridgeAdd );
  pMultipart->addtyp<msg::header>( hdrSnd );

  pMultipart->addstr( uuidSwitch );
  pMultipart->addstr( uuidBridge );

  PostToZmqRequest( pMultipart );
}

void Control::HandleBridgeUpdate( const ovsdb::uuidBridge_t& uuidBridge, const ovsdb::bridge_t& br ) {
  // ovs -> local (via request):
  auto pMultipart = std::make_unique<zmq::multipart_t>();  // TODO: use a pool?

  msg::header hdrSnd( 1, msg::type::eOvsBridgeUpdate );
  pMultipart->addtyp<msg::header>( hdrSnd );

  pMultipart->addstr( uuidBridge );
  pMultipart->addstr( br.name );
  pMultipart->addstr( br.datapath_id );

  PostToZmqRequest( pMultipart );
}

void Control::HandleBridgeDelete( const ovsdb::uuidBridge_t& uuidBridge ) {
}

void Control::HandlePortAdd( const ovsdb::uuid_t& uuidBridge, const ovsdb::uuidPort_t& uuidPort ) {
  // ovs -> local (via request):
  auto pMultipart = std::make_unique<zmq::multipart_t>();  // TODO: use a pool?

  msg::header hdrSnd( 1, msg::type::eOvsPortAdd );
  pMultipart->addtyp<msg::header>( hdrSnd );

  pMultipart->addstr( uuidBridge );
  pMultipart->addstr( uuidPort );

  PostToZmqRequest( pMultipart );
}

void Control::HandlePortUpdate( const ovsdb::uuidPort_t& uuidPort, const ovsdb::port_t& port ) {
  // ovs -> local (via request):
  auto pMultipart = std::make_unique<zmq::multipart_t>();  // TODO: use a pool?

  msg::header hdrSnd( 1, msg::type::eOvsPortUpdate );
  pMultipart->addtyp<msg::header>( hdrSnd );

  pMultipart->addstr( uuidPort );
  pMultipart->addtyp<uint16_t>( port.tag );
  pMultipart->addtyp<uint16_t>( port.setTrunk.size() );
  for ( auto item: port.setTrunk ) {
    pMultipart->addtyp<uint16_t>( item );
  }

  PostToZmqRequest( pMultipart );
}

void Control::HandlePortDelete( const ovsdb::uuidPort_t& uuidPort ) {
}

void Control::HandleInterfaceAdd( const ovsdb::uuidPort_t& uuidPort, const ovsdb::uuidInterface_t& uuidInterface ) {
  // ovs -> local (via request):
  auto pMultipart = std::make_unique<zmq::multipart_t>();  // TODO: use a pool?

  msg::header hdrSnd( 1, msg::type::eOvsInterfaceAdd );
  pMultipart->addtyp<msg::header>( hdrSnd );

  pMultipart->addstr( uuidPort );
  pMultipart->addstr( uuidInterface );

  PostToZmqRequest( pMultipart );
}

void Control::HandleInterfaceUpdate( const ovsdb::uuidInterface_t& uuidInterface, const ovsdb::interface_t& interface) {
  // ovs -> local (via request):
  auto pMultipart = std::make_unique<zmq::multipart_t>();  // TODO: use a pool?

  msg::header hdrSnd( 1, msg::type::eOvsInterfaceUpdate );
  pMultipart->addtyp<msg::header>( hdrSnd );

  pMultipart->addstr( uuidInterface );
  pMultipart->addstr( interface.name );
  pMultipart->addstr( interface.ovs_type );
  pMultipart->addstr( interface.admin_state );
  pMultipart->addstr( interface.link_state );
  pMultipart->addstr( interface.mac_in_use );

  PostToZmqRequest( pMultipart );
}

void Control::HandleInterfaceDelete( const ovsdb::uuidInterface_t& uuidInterface ) {
}

void Control::HandleStatisticsUpdate( const ovsdb::uuidInterface_t& uuidInterface, const ovsdb::statistics_t& stats ) {
  // ovs -> local (via request):
  auto pMultipart = std::make_unique<zmq::multipart_t>();  // TODO: use a pool?

  msg::header hdrSnd( 1, msg::type::eOvsInterfaceStatistics );
  pMultipart->addtyp<msg::header>( hdrSnd );

  pMultipart->addstr( uuidInterface );
  pMultipart->addtyp<size_t>( stats.collisions );
  pMultipart->addtyp<size_t>( stats.rx_bytes );
  pMultipart->addtyp<size_t>( stats.rx_crc_err );
  pMultipart->addtyp<size_t>( stats.rx_dropped );
  pMultipart->addtyp<size_t>( stats.rx_errors );
  pMultipart->addtyp<size_t>( stats.rx_frame_err );
  pMultipart->addtyp<size_t>( stats.rx_over_err );
  pMultipart->addtyp<size_t>( stats.rx_packets );
  pMultipart->addtyp<size_t>( stats.tx_bytes );
  pMultipart->addtyp<size_t>( stats.tx_dropped );
  pMultipart->addtyp<size_t>( stats.tx_errors );
  pMultipart->addtyp<size_t>( stats.collisions );

  PostToZmqRequest( pMultipart );
}
