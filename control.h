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
  typedef std::shared_ptr<zmq::multipart_t> pMultipart_t;

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

  void PostToZmqRequest( pMultipart_t );

  void HandleSwitchAdd( const ovsdb::uuid_t& uuid );
  void HandleSwitchUpdate( const ovsdb::uuid_t& uuid, const ovsdb::switch_t& );
  void HandleSwitchDelete( const ovsdb::uuid_t& uuid );

  void HandleBridgeAdd( const ovsdb::uuid_t& uuid );
  void HandleBridgeUpdate( const ovsdb::uuid_t& uuid, const ovsdb::bridge_t& );
  void HandleBridgeDelete( const ovsdb::uuid_t& uuid );

  void HandlePortAdd( const ovsdb::uuid_t& uuid );
  void HandlePortUpdate( const ovsdb::uuid_t& uuid, const ovsdb::port_t& );
  void HandlePortDelete( const ovsdb::uuid_t& uuid );

  void HandleInterfaceAdd( const ovsdb::uuid_t& uuid );
  void HandleInterfaceUpdate( const ovsdb::uuid_t& uuid, const ovsdb::interface_t& );
  void HandleInterfaceDelete( const ovsdb::uuid_t& uuid );

  void HandleStatisticsUpdate( const ovsdb::uuid_t& uuid, const ovsdb::statistics_t& );

};

#endif /* CONTROL_H */
