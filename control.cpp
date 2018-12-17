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
   ovs-vsctl set-controller ovsbr0 tcp:127.0.0.1:6633 # keep local
   ip link set dev ovsbr0 up
   ip link set dev ovs-system up
 *
 * other commands:
    ovs-vsctl set-fail-mode ovsbr0 secure|standalone  // standalone is default, or empty
    //ovs-vsctl del-fail-mode ovsbr0
    ovs-vsctl get-fail-mode ovsbr0
 */

#include <memory>
#include <iostream>

#include <boost/asio/post.hpp>

#include <boost/log/trivial.hpp>

#include "../quadlii/lib/common/monitor_t.h"
#include "../quadlii/lib/common/ZmqMessage.h"

#include "ovsdb.h"
#include "control.h"
#include "tcp_session.h"
#include "protocol/ethernet/address.h"

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
        exit(0);
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

    ovsdb::structures::f_t f;

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

    ovsdb::decode m_ovsdb( m_ioContext, f );

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
      try {
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
      }
      catch (...) {
        BOOST_LOG_TRIVIAL(trace) << "Control::PostToZmqRequest asio::post problems";
      }
    } );
  }
  catch (...) {
    BOOST_LOG_TRIVIAL(trace) << "Control::PostToZmqRequest problems";
  }
}

// TODO: put these structures into a database?  or just run live from the updates obtained during startup?
// TOOD: maybe auto-push like ovsdb does on new connections?  but this won't work unless there is a message from the subscribe queue
//   this then allows refactoring the below to message passing, local storage, and mesasge generation from existing structures

void Control::HandleSwitchAdd( const ovsdb::structures::uuidSwitch_t& uuidSwitch ) {
  HandleSwitchAdd_local( uuidSwitch );
  HandleSwitchAdd_msg( uuidSwitch );
}

void Control::HandleSwitchAdd_local( const ovsdb::structures::uuidSwitch_t& uuidSwitch ) {
  mapSwitch_t::iterator iterSwitch = m_mapSwitch.find( uuidSwitch );
  if ( m_mapSwitch.end() == iterSwitch ) {
    iterSwitch = m_mapSwitch.insert( m_mapSwitch.begin(), mapSwitch_t::value_type( uuidSwitch, switch_composite_t() ) );
    assert( m_mapSwitch.end() != iterSwitch );
  }
}

void Control::HandleSwitchAdd_msg( const ovsdb::structures::uuidSwitch_t& uuidSwitch ) {
  auto pMultipart = std::make_unique<zmq::multipart_t>();  // TODO: use a pool?

  msg::header hdrSnd( 1, msg::type::eOvsSwitchAdd );
  pMultipart->addtyp<msg::header>( hdrSnd );

  pMultipart->addstr( uuidSwitch );

  PostToZmqRequest( pMultipart );
}

// ==

void Control::HandleSwitchUpdate( const ovsdb::structures::uuidSwitch_t& uuidSwitch, const ovsdb::structures::switch_t& sw ) {
  HandleSwitchUpdate_local( uuidSwitch, sw );
  HandleSwitchUpdate_msg( uuidSwitch, sw );
}

void Control::HandleSwitchUpdate_local( const ovsdb::structures::uuidSwitch_t& uuidSwitch, const ovsdb::structures::switch_t&  sw ) {
  mapSwitch_t::iterator iterSwitch = m_mapSwitch.find( uuidSwitch );
  if ( m_mapSwitch.end() == iterSwitch ) {
    BOOST_LOG_TRIVIAL(warning) << "Control::HandleSwitchUpdate switch " << uuidSwitch << " does not exist";
  }
  else {
    switch_composite_t& switch_( iterSwitch->second );
    switch_.sw = sw;
  }
}

void Control::HandleSwitchUpdate_msg( const ovsdb::structures::uuidSwitch_t& uuidSwitch, const ovsdb::structures::switch_t&  sw ) {
  auto pMultipart = std::make_unique<zmq::multipart_t>();  // TODO: use a pool?

  msg::header hdrSnd( 1, msg::type::eOvsSwitchUpdate );
  pMultipart->addtyp<msg::header>( hdrSnd );

  pMultipart->addstr( uuidSwitch );
  pMultipart->addstr( sw.hostname );
  pMultipart->addstr( sw.ovs_version );
  pMultipart->addstr( sw.db_version );

  PostToZmqRequest( pMultipart );
}

// ==

void Control::HandleSwitchDelete( const ovsdb::structures::uuidSwitch_t& uuidSwitch ) {
  HandleSwitchDelete_local( uuidSwitch );
  HandleSwitchDelete_msg( uuidSwitch );
}

void Control::HandleSwitchDelete_local( const ovsdb::structures::uuidSwitch_t& uuidSwitch ) {
  mapSwitch_t::iterator iterSwitch = m_mapSwitch.find( uuidSwitch );
  if ( m_mapSwitch.end() == iterSwitch ) {
    BOOST_LOG_TRIVIAL(warning) << "Control::HandleSwitchDelete switch " << uuidSwitch << " does not exist";
  }
  else {
    assert( iterSwitch->second.setBridge.empty() );
    m_mapSwitch.erase( iterSwitch );
    BOOST_LOG_TRIVIAL(info) << "Control::HandleSwitchDelete switch " << uuidSwitch << " deleted";
  }
}

void Control::HandleSwitchDelete_msg( const ovsdb::structures::uuidSwitch_t& uuidSwitch ) {
}

// ==

void Control::HandleBridgeAdd( const ovsdb::structures::uuidSwitch_t& uuidSwitch, const ovsdb::structures::uuid_t& uuidBridge ) {
  HandleBridgeAdd_local( uuidSwitch, uuidBridge );
  HandleBridgeAdd_msg( uuidSwitch, uuidBridge );
}

void Control::HandleBridgeAdd_local( const ovsdb::structures::uuidSwitch_t& uuidSwitch, const ovsdb::structures::uuidBridge_t& uuidBridge )  {
  mapSwitch_t::iterator iterSwitch = m_mapSwitch.find( uuidSwitch );
  if ( m_mapSwitch.end() == iterSwitch ) {
    BOOST_LOG_TRIVIAL(warning) << "Control::HandleBridgeAdd switch " << uuidSwitch << " does not exist";
  }
  else {
    switch_composite_t switch_( iterSwitch->second );
    setSwitch_t::iterator iterSetSwitch = switch_.setBridge.find( uuidBridge );
    if ( switch_.setBridge.end() == iterSetSwitch ) {
      switch_.setBridge.insert( setBridge_t::value_type( uuidBridge ) );
      mapBridge_t::iterator iterBridge = m_mapBridge.insert( m_mapBridge.begin(), mapBridge_t::value_type( uuidBridge, bridge_composite_t() ) );
      assert( m_mapBridge.end() != iterBridge );
      iterBridge->second.uuidOwnerSwitch = uuidSwitch;
    }
  }
}

void Control::HandleBridgeAdd_msg( const ovsdb::structures::uuidSwitch_t& uuidSwitch, const ovsdb::structures::uuidBridge_t& uuidBridge ) {
  auto pMultipart = std::make_unique<zmq::multipart_t>();  // TODO: use a pool?

  msg::header hdrSnd( 1, msg::type::eOvsBridgeAdd );
  pMultipart->addtyp<msg::header>( hdrSnd );

  pMultipart->addstr( uuidSwitch );
  pMultipart->addstr( uuidBridge );

  PostToZmqRequest( pMultipart );
}

// ==

void Control::HandleBridgeUpdate( const ovsdb::structures::uuidBridge_t& uuidBridge, const ovsdb::structures::bridge_t& br ) {
  HandleBridgeUpdate_local( uuidBridge, br );
  HandleBridgeUpdate_msg( uuidBridge, br );
}

void Control::HandleBridgeUpdate_local( const ovsdb::structures::uuidBridge_t& uuidBridge, const ovsdb::structures::bridge_t& br) {
  mapBridge_t::iterator iterBridge = m_mapBridge.find( uuidBridge );
  if ( m_mapBridge.end() == iterBridge ) {
    BOOST_LOG_TRIVIAL(warning) << "Control::HandleBridgeUpdate bridge " << uuidBridge << " does not exist";
  }
  else {
    bridge_composite_t& bridge( iterBridge->second );
    bridge.br = br;
  }
}

void Control::HandleBridgeUpdate_msg( const ovsdb::structures::uuidBridge_t& uuidBridge, const ovsdb::structures::bridge_t& br ) {
  auto pMultipart = std::make_unique<zmq::multipart_t>();  // TODO: use a pool?

  msg::header hdrSnd( 1, msg::type::eOvsBridgeUpdate );
  pMultipart->addtyp<msg::header>( hdrSnd );

  pMultipart->addstr( uuidBridge );
  pMultipart->addstr( br.name );
  pMultipart->addstr( br.datapath_id );

  PostToZmqRequest( pMultipart );
}

// ==

void Control::HandleBridgeDelete( const ovsdb::structures::uuidBridge_t& uuidBridge ) {
  HandleBridgeDelete_local( uuidBridge );
  HandleBridgeDelete_msg( uuidBridge );
}

void Control::HandleBridgeDelete_local( const ovsdb::structures::uuidBridge_t& uuidBridge ) {
  mapBridge_t::iterator iterMapBridge = m_mapBridge.find( uuidBridge );
  if ( m_mapBridge.end() == iterMapBridge ) {
    BOOST_LOG_TRIVIAL(warning) << "Control::HandleBridgeDelete bridge " << uuidBridge << " map item does not exist";
  }
  else {
    mapSwitch_t::iterator iterSwitch = m_mapSwitch.find( iterMapBridge->second.uuidOwnerSwitch );
    assert( m_mapSwitch.end() != iterSwitch );
    setBridge_t::iterator iterSetBridge = iterSwitch->second.setBridge.find( uuidBridge );
    assert( iterSwitch->second.setBridge.end() != iterSetBridge );
    iterSwitch->second.setBridge.erase( iterSetBridge );

    m_mapBridge.erase( iterMapBridge );
    BOOST_LOG_TRIVIAL(info) << "Control::HandleBridgeDelete bridge" << uuidBridge << " deleted";
  }
}

void Control::HandleBridgeDelete_msg( const ovsdb::structures::uuidBridge_t& ) {
}

// ==

void Control::HandlePortAdd( const ovsdb::structures::uuid_t& uuidBridge, const ovsdb::structures::uuidPort_t& uuidPort ) {
  HandlePortAdd_local( uuidBridge, uuidPort );
  HandlePortAdd_msg( uuidBridge, uuidPort );
}

void Control::HandlePortAdd_local( const ovsdb::structures::uuidBridge_t& uuidBridge, const ovsdb::structures::uuidPort_t& uuidPort ) {
  mapBridge_t::iterator iterMapBridge = m_mapBridge.find( uuidBridge );
  if ( m_mapBridge.end() == iterMapBridge ) {
    BOOST_LOG_TRIVIAL(warning) << "Control::HandlePortAdd bridge " << uuidBridge << " does not exist";
  }
  else {
    bridge_composite_t bridge( iterMapBridge->second );
    setPort_t::iterator iterSetPort = bridge.setPort.find( uuidPort );
    if ( bridge.setPort.end() == iterSetPort ) {
      bridge.setPort.insert( setPort_t::value_type( uuidPort ) );
      mapPort_t::iterator iterPort = m_mapPort.insert( m_mapPort.begin(), mapPort_t::value_type( uuidPort, port_composite_t() ) );
      assert( m_mapPort.end() != iterPort );
      iterPort->second.uuidOwnerBridge = uuidBridge;
    }
  }
}

void Control::HandlePortAdd_msg( const ovsdb::structures::uuidBridge_t& uuidBridge, const ovsdb::structures::uuidPort_t& uuidPort ) {
  auto pMultipart = std::make_unique<zmq::multipart_t>();  // TODO: use a pool?

  msg::header hdrSnd( 1, msg::type::eOvsPortAdd );
  pMultipart->addtyp<msg::header>( hdrSnd );

  pMultipart->addstr( uuidBridge );
  pMultipart->addstr( uuidPort );

  PostToZmqRequest( pMultipart );
}

// ==

void Control::HandlePortUpdate( const ovsdb::structures::uuidPort_t& uuidPort, const ovsdb::structures::port_t& port ) {
  HandlePortUpdate_local( uuidPort, port );
  HandlePortUpdate_msg( uuidPort, port );
}

void Control::HandlePortUpdate_local( const ovsdb::structures::uuidPort_t& uuidPort, const ovsdb::structures::port_t& port ) {
  mapPort_t::iterator iterPort = m_mapPort.find( uuidPort );
  if ( m_mapPort.end() == iterPort ) {
    BOOST_LOG_TRIVIAL(warning) << "Control::HandlePortUpdate port " << uuidPort << " does not exist";
  }
  else {
    port_composite_t& port_( iterPort->second );
    port_.port = port;
  }
}

void Control::HandlePortUpdate_msg( const ovsdb::structures::uuidPort_t& uuidPort, const ovsdb::structures::port_t& port ) {
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

// ==

void Control::HandlePortDelete( const ovsdb::structures::uuidPort_t& uuidPort ) {
  HandlePortDelete_local( uuidPort );
  HandlePortDelete_msg( uuidPort );
}

void Control::HandlePortDelete_local( const ovsdb::structures::uuidPort_t& uuidPort ) {
  mapPort_t::iterator iterMapPort = m_mapPort.find( uuidPort );
  if ( m_mapPort.end() == iterMapPort ) {
    BOOST_LOG_TRIVIAL(warning) << "Control::HandlePortDelete port " << uuidPort << " map item does not exist";
  }
  else {
    mapBridge_t::iterator iterBridge = m_mapBridge.find( iterMapPort->second.uuidOwnerBridge );
    assert( m_mapBridge.end() != iterBridge );
    setPort_t::iterator iterSetPort = iterBridge->second.setPort.find( uuidPort );
    assert( iterBridge->second.setPort.end() != iterSetPort );
    iterBridge->second.setPort.erase( iterSetPort );

    m_mapPort.erase( iterMapPort );
    BOOST_LOG_TRIVIAL(info) << "Control::HandlePortDelete port" << uuidPort << " deleted";
  }
}

void Control::HandlePortDelete_msg( const ovsdb::structures::uuidPort_t& uuidPort ) {
}

// ==

void Control::HandleInterfaceAdd( const ovsdb::structures::uuidPort_t& uuidPort, const ovsdb::structures::uuidInterface_t& uuidInterface ) {
  HandleInterfaceAdd_local( uuidPort, uuidInterface );
  HandleInterfaceAdd_msg( uuidPort, uuidInterface );
}

void Control::HandleInterfaceAdd_local( const ovsdb::structures::uuidPort_t& uuidPort, const ovsdb::structures::uuidInterface_t& uuidInterface) {
  mapPort_t::iterator iterMapPort = m_mapPort.find( uuidPort );
  if ( m_mapPort.end() == iterMapPort ) {
    BOOST_LOG_TRIVIAL(warning) << "Control::HandleInterfaceAdd port " << uuidPort << " does not exist";
  }
  else {
    port_composite_t port( iterMapPort->second );
    setInterface_t::iterator iterSetInterface = port.setInterface.find( uuidInterface );
    if ( port.setInterface.end() == iterSetInterface ) {
      port.setInterface.insert( setInterface_t::value_type( uuidInterface ) );
      mapInterface_t::iterator iterInterface = m_mapInterface.insert( m_mapInterface.begin(), mapInterface_t::value_type( uuidInterface, interface_composite_t() ) );
      assert( m_mapInterface.end() != iterInterface );
      iterInterface->second.uuidOwnerPort = uuidPort;
    }
  }

}

void Control::HandleInterfaceAdd_msg( const ovsdb::structures::uuidPort_t& uuidPort, const ovsdb::structures::uuidInterface_t& uuidInterface ) {
  auto pMultipart = std::make_unique<zmq::multipart_t>();  // TODO: use a pool?

  msg::header hdrSnd( 1, msg::type::eOvsInterfaceAdd );
  pMultipart->addtyp<msg::header>( hdrSnd );

  pMultipart->addstr( uuidPort );
  pMultipart->addstr( uuidInterface );

  PostToZmqRequest( pMultipart );
}

// ==

void Control::HandleInterfaceUpdate( const ovsdb::structures::uuidInterface_t& uuidInterface, const ovsdb::structures::interface_t& interface) {
  HandleInterfaceUpdate_local( uuidInterface, interface );
  HandleInterfaceUpdate_msg( uuidInterface, interface );
}

void Control::HandleInterfaceUpdate_local( const ovsdb::structures::uuidInterface_t& uuidInterface, const ovsdb::structures::interface_t& interface ) {
  mapInterface_t::iterator iterInterface = m_mapInterface.find( uuidInterface );
  if ( m_mapInterface.end() == iterInterface ) {
    BOOST_LOG_TRIVIAL(warning) << "Control::HandleInterfaceUpdate interface " << uuidInterface << " does not exist";
  }
  else {
    interface_composite_t& interface_( iterInterface->second );
    interface_.interface = interface;

    mapPort_t::const_iterator iterPort = m_mapPort.find( interface_.uuidOwnerPort );
    const ovsdb::structures::port_t& port( iterPort->second.port );

    Bridge::interface_t bi;

    bi.tag = port.tag;
    bi.setTrunk = port.setTrunk;
    bi.ifindex = interface.ifindex;
    bi.ofport = interface.ofport;

    if ( "up" == interface.admin_state ) {
      bi.admin_state = Bridge::OpState::up;
    }
    else {
      if ( "down" == interface.admin_state ) {
        bi.admin_state = Bridge::OpState::down;
      }
      else {
        std::cout << "interface.admin_state unknown: " << interface.admin_state << std::endl;
      }
    }

    if ( "up" == interface.link_state ) {
      bi.link_state = Bridge::OpState::up;
    }
    else {
      if ( "down" == interface.link_state ) {
        bi.link_state = Bridge::OpState::down;
      }
      else {
        std::cout << "interface.link_state unknown: " << interface.link_state << std::endl;
      }
    }

    // TODO: put a spirit parser on this
    if ( 0 == port.VlanMode.size() ) {
      if ( ( 0 == port.tag ) && ( port.setTrunk.empty() ) ) {
        bi.eVlanMode = Bridge::VlanMode::trunk;
      }
      else {
        if ( ( 0 != port.tag ) && ( !port.setTrunk.empty() ) ) {
          bi.eVlanMode = Bridge::VlanMode::native_tagged;
        }
        else {
          if ( 0 != port.tag ) {
            bi.eVlanMode = Bridge::VlanMode::access;
          }
          else {
            bi.eVlanMode = Bridge::VlanMode::trunk; // setTrunk is empty(all vlans) or populated(selected vlans)
          }
        }
      }
    }
    else {
      if ( "native-tagged" == port.VlanMode ) bi.eVlanMode = Bridge::VlanMode::native_tagged;
      else {
        if ( "dot1q-tunnel" == port.VlanMode ) bi.eVlanMode = Bridge::VlanMode::dot1q_tunnel;
        else {
          if ( "trunk" == port.VlanMode ) bi.eVlanMode = Bridge::VlanMode::trunk;
          else {
            if ( "access" == port.VlanMode ) bi.eVlanMode = Bridge::VlanMode::access;
            else {
              if ( "native-untagged" == port.VlanMode ) bi.eVlanMode = Bridge::VlanMode::native_untagged;
              else {
                std::cout << "Control::HandleInterfaceUpdate_local - unknown vlan_mode: " << port.VlanMode << std::endl;
                assert( 0 );
              }
            }
          }
        }
      }
    }

    protocol::ethernet::ConvertStringToMac( interface.mac_in_use, bi.mac_in_use );

    m_bridge.UpdateInterface( bi );

  }
}

void Control::HandleInterfaceUpdate_msg( const ovsdb::structures::uuidInterface_t& uuidInterface, const ovsdb::structures::interface_t& interface ) {
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

// ==

void Control::HandleInterfaceDelete( const ovsdb::structures::uuidInterface_t& uuidInterface ) {
  HandleInterfaceDelete_local( uuidInterface );
  HandleInterfaceDelete_msg( uuidInterface );
}

void Control::HandleInterfaceDelete_local( const ovsdb::structures::uuidInterface_t& uuidInterface ) {
  mapInterface_t::iterator iterMapInterface = m_mapInterface.find( uuidInterface );
  if ( m_mapInterface.end() == iterMapInterface ) {
    BOOST_LOG_TRIVIAL(warning) << "Control::HandleInterfaceDelete interface " << uuidInterface << " map item does not exist";
  }
  else {
    mapPort_t::iterator iterPort = m_mapPort.find( iterMapInterface->second.uuidOwnerPort );
    assert( m_mapPort.end() != iterPort );
    setInterface_t::iterator iterSetInterface = iterPort->second.setInterface.find( uuidInterface );
    assert( iterPort->second.setInterface.end() != iterSetInterface );
    iterPort->second.setInterface.erase( iterSetInterface );

    m_mapInterface.erase( iterMapInterface );
    BOOST_LOG_TRIVIAL(info) << "Control::HandleInterfaceDelete interface" << uuidInterface << " deleted";
  }
}

void Control::HandleInterfaceDelete_msg( const ovsdb::structures::uuidInterface_t& uuidInterface ) {
}

// ==

void Control::HandleStatisticsUpdate( const ovsdb::structures::uuidInterface_t& uuidInterface, const ovsdb::structures::statistics_t& stats ) {
  HandleStatisticsUpdate_local( uuidInterface, stats );
  HandleStatisticsUpdate_msg( uuidInterface, stats );
}

void Control::HandleStatisticsUpdate_local( const ovsdb::structures::uuidInterface_t& uuidInterface, const ovsdb::structures::statistics_t& stats ) {
  mapInterface_t::iterator iterInterface = m_mapInterface.find( uuidInterface );
  if ( m_mapInterface.end() == iterInterface ) {
    BOOST_LOG_TRIVIAL(warning) << "Control::HandleStatisticsUpdate interface " << uuidInterface << " does not exist";
  }
  else {
    interface_composite_t& interface_( iterInterface->second );
    interface_.stats = stats;
  }
}

void Control::HandleStatisticsUpdate_msg( const ovsdb::structures::uuidInterface_t& uuidInterface, const ovsdb::structures::statistics_t& stats ) {
  auto pMultipart = std::make_unique<zmq::multipart_t>();  // TODO: use a pool?

  msg::header hdrSnd( 1, msg::type::eOvsInterfaceStatistics );
  pMultipart->addtyp<msg::header>( hdrSnd );

  pMultipart->addstr( uuidInterface );
  // TODO: convert to a single typed structure of statistics_t (to be done in network byte order?)
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
  pMultipart->addtyp<size_t>( stats.tx_packets );

  PostToZmqRequest( pMultipart );
}
