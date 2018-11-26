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
#include "ovsdb.h"

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

  void AcceptControlConnections();

  void PostToZmqRequest( pMultipart_t& );

  void    HandleSwitchAdd( const ovsdb::structures::uuidSwitch_t& );
  void HandleSwitchUpdate( const ovsdb::structures::uuidSwitch_t&, const ovsdb::structures::switch_t& );
  void HandleSwitchDelete( const ovsdb::structures::uuidSwitch_t& );

  void    HandleBridgeAdd( const ovsdb::structures::uuidSwitch_t&, const ovsdb::structures::uuidBridge_t& );
  void HandleBridgeUpdate( const ovsdb::structures::uuidBridge_t&, const ovsdb::structures::bridge_t& );
  void HandleBridgeDelete( const ovsdb::structures::uuidBridge_t& );

  void    HandlePortAdd( const ovsdb::structures::uuidBridge_t&, const ovsdb::structures::uuidPort_t& );
  void HandlePortUpdate( const ovsdb::structures::uuidPort_t&, const ovsdb::structures::port_t& );
  void HandlePortDelete( const ovsdb::structures::uuidPort_t& );

  void    HandleInterfaceAdd( const ovsdb::structures::uuidPort_t&, const ovsdb::structures::uuidInterface_t& );
  void HandleInterfaceUpdate( const ovsdb::structures::uuidInterface_t&, const ovsdb::structures::interface_t& );
  void HandleInterfaceDelete( const ovsdb::structures::uuidInterface_t& );

  void HandleStatisticsUpdate( const ovsdb::structures::uuidInterface_t&, const ovsdb::structures::statistics_t& );

};

#endif /* CONTROL_H */
