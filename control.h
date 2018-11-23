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
  
  ovsdb::f_t m_f;
  
  ovsdb m_ovsdb;
  
  void AcceptControlConnections();
};

#endif /* CONTROL_H */

