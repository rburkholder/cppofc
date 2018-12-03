/*
 * File:   tcp_session.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on June 6, 2017, 11:07 AM
 */

#ifndef TCP_SESSION_H
#define TCP_SESSION_H

#include <queue>
#include <mutex>
#include <atomic>

#include <boost/asio/ip/tcp.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "common.h"
#include "Buffer.h"
#include "bridge.h"

namespace asio = boost::asio;
namespace ip = asio::ip;

class tcp_session
  : public std::enable_shared_from_this<tcp_session>
{
public:

  tcp_session( Bridge& bridge, ip::tcp::socket socket);
  virtual ~tcp_session();

  void start();

private:

  enum { max_length = 65560 };  // total header and data for ipv4 is 65535

// need to use a function object instead so that the functions are embedded.
// can be stack based function object or a heap based function object
// supplied by the primary data structure being built
// then can embed the related dependencies for building the various fields.
  struct build {
    typedef std::function<size_t(void)> fSize_t;
    typedef std::function<void(vByte_t&)> fAppend_t;
  };

  std::vector<build> m_vBuild; // used for building up a packet from composite structures.

  void do_read();

  //void do_write(std::size_t length);

  // TODO:  need a write queue, need to update the xid value in the header
  //    so, create a method or class for handling queued messages and transactions
  //void do_write( vChar_t& v );
  void do_write();

  ip::tcp::socket m_socket;

  vByte_t m_vRx;
  vByte_t m_vReassembly;
  typedef vByte_t::iterator vByte_iter_t;

  typedef std::queue<vByte_t> qBuffers_t;

  // TODO: run stuff using these constructs through a strand instead
  std::atomic<uint32_t> m_transmitting;

  //qBuffers_t m_qBuffersAvailable;
  //qBuffers_t m_qTxBuffersToBeWritten;
  vByte_t m_vTxInWrite;

  Buffer m_bufferAvailable;
  Buffer m_bufferWaitingToTx;

  Bridge& m_bridge;

  //asio::io_context::strand m_ioStrand;

  void ProcessPacket( uint8_t* pBegin, const uint8_t* pEnd );

  void GetAvailableBuffer( vByte_t& v );
  vByte_t GetAvailableBuffer();
  void QueueTxToWrite( vByte_t );
  void LoadTxInWrite();
  void UnloadTxInWrite();

};


#endif /* TCP_SESSION_H */

