/*
 * File:   control.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net *
 *
 * Created on November 22, 2018, 4:17 PM
 */

#ifndef CONTROL_H
#define CONTROL_H

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/signal_set.hpp>

#include <boost/thread/thread.hpp>

#include <zmq.hpp>
#include <zmq_addon.hpp>

#include "bridge.h"
#include "ovsdb_structures.h"

namespace asio = boost::asio;
namespace ip = asio::ip;

class Control {
public:
  Control( int port );
  virtual ~Control();
  void Start();
protected:
private:

  typedef asio::executor_work_guard<asio::io_context::executor_type> io_context_work;
  typedef std::unique_ptr<zmq::multipart_t> pMultipart_t;

  int m_port;

  asio::io_context m_ioContext;
  asio::io_context::strand m_strandZmqRequest;  // strand for cppof->local messages
  boost::asio::signal_set m_signals;

  io_context_work m_ioWork;

  boost::thread_group m_threads;

  ip::tcp::acceptor m_acceptor;
  ip::tcp::socket m_socket;

  zmq::context_t m_zmqContext;
  zmq::socket_t m_zmqSocketRequest;

  Bridge m_bridge;

  typedef ovsdb::structures::uuidSwitch_t uuidSwitch_t;
  typedef ovsdb::structures::uuidBridge_t uuidBridge_t;
  typedef ovsdb::structures::uuidPort_t uuidPort_t;
  typedef ovsdb::structures::uuidInterface_t uuidInterface_t;

  typedef ovsdb::structures::mapSwitch_t mapSwitch_t;
  typedef ovsdb::structures::mapBridge_t mapBridge_t;
  typedef ovsdb::structures::mapPort_t mapPort_t;
  typedef ovsdb::structures::mapInterface_t mapInterface_t;

  ovsdb::structures::mapSwitch_t m_mapSwitch;
  ovsdb::structures::mapBridge_t m_mapBridge;
  ovsdb::structures::mapPort_t m_mapPort;
  ovsdb::structures::mapInterface_t m_mapInterface;

  void AcceptControlConnections();

  void PostToZmqRequest( pMultipart_t& );

  void HandleSwitchAdd( const ovsdb::structures::uuidSwitch_t& );
  void HandleSwitchAdd_local( const ovsdb::structures::uuidSwitch_t& );
  void HandleSwitchAdd_msg( const ovsdb::structures::uuidSwitch_t& );

  void HandleSwitchUpdate( const ovsdb::structures::uuidSwitch_t&, const ovsdb::structures::switch_t& );
  void HandleSwitchUpdate_local( const ovsdb::structures::uuidSwitch_t&, const ovsdb::structures::switch_t& );
  void HandleSwitchUpdate_msg( const ovsdb::structures::uuidSwitch_t&, const ovsdb::structures::switch_t& );

  void HandleSwitchDelete( const ovsdb::structures::uuidSwitch_t& );
  void HandleSwitchDelete_local( const ovsdb::structures::uuidSwitch_t& );
  void HandleSwitchDelete_msg( const ovsdb::structures::uuidSwitch_t& );

  void HandleBridgeAdd( const ovsdb::structures::uuidSwitch_t&, const ovsdb::structures::uuidBridge_t& );
  void HandleBridgeAdd_local( const ovsdb::structures::uuidSwitch_t&, const ovsdb::structures::uuidBridge_t& );
  void HandleBridgeAdd_msg( const ovsdb::structures::uuidSwitch_t&, const ovsdb::structures::uuidBridge_t& );

  void HandleBridgeUpdate( const ovsdb::structures::uuidBridge_t&, const ovsdb::structures::bridge_t& );
  void HandleBridgeUpdate_local( const ovsdb::structures::uuidBridge_t&, const ovsdb::structures::bridge_t& );
  void HandleBridgeUpdate_msg( const ovsdb::structures::uuidBridge_t&, const ovsdb::structures::bridge_t& );

  void HandleBridgeDelete( const ovsdb::structures::uuidBridge_t& );
  void HandleBridgeDelete_local( const ovsdb::structures::uuidBridge_t& );
  void HandleBridgeDelete_msg( const ovsdb::structures::uuidBridge_t& );

  void HandlePortAdd( const ovsdb::structures::uuidBridge_t&, const ovsdb::structures::uuidPort_t& );
  void HandlePortAdd_local( const ovsdb::structures::uuidBridge_t&, const ovsdb::structures::uuidPort_t& );
  void HandlePortAdd_msg( const ovsdb::structures::uuidBridge_t&, const ovsdb::structures::uuidPort_t& );

  void HandlePortUpdate( const ovsdb::structures::uuidPort_t&, const ovsdb::structures::port_t& );
  void HandlePortUpdate_local( const ovsdb::structures::uuidPort_t&, const ovsdb::structures::port_t& );
  void HandlePortUpdate_msg( const ovsdb::structures::uuidPort_t&, const ovsdb::structures::port_t& );

  void HandlePortDelete( const ovsdb::structures::uuidPort_t& );
  void HandlePortDelete_local( const ovsdb::structures::uuidPort_t& );
  void HandlePortDelete_msg( const ovsdb::structures::uuidPort_t& );

  void HandleInterfaceAdd( const ovsdb::structures::uuidPort_t&, const ovsdb::structures::uuidInterface_t& );
  void HandleInterfaceAdd_local( const ovsdb::structures::uuidPort_t&, const ovsdb::structures::uuidInterface_t& );
  void HandleInterfaceAdd_msg( const ovsdb::structures::uuidPort_t&, const ovsdb::structures::uuidInterface_t& );

  void HandleInterfaceUpdate( const ovsdb::structures::uuidInterface_t&, const ovsdb::structures::interface_t& );
  void HandleInterfaceUpdate_local( const ovsdb::structures::uuidInterface_t&, const ovsdb::structures::interface_t& );
  void HandleInterfaceUpdate_msg( const ovsdb::structures::uuidInterface_t&, const ovsdb::structures::interface_t& );

  void HandleInterfaceDelete( const ovsdb::structures::uuidInterface_t& );
  void HandleInterfaceDelete_local( const ovsdb::structures::uuidInterface_t& );
  void HandleInterfaceDelete_msg( const ovsdb::structures::uuidInterface_t& );

  void HandleStatisticsUpdate( const ovsdb::structures::uuidInterface_t&, const ovsdb::structures::statistics_t& );
  void HandleStatisticsUpdate_local( const ovsdb::structures::uuidInterface_t&, const ovsdb::structures::statistics_t& );
  void HandleStatisticsUpdate_msg( const ovsdb::structures::uuidInterface_t&, const ovsdb::structures::statistics_t& );

};

#endif /* CONTROL_H */
