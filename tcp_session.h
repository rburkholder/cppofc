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

//#include <boost/asio/io_context.hpp>
//#include <boost/asio/post.hpp>
//#include <boost/asio/strand.hpp>
//#include <boost/thread/thread.hpp>

//#include <zmq.hpp>
//#include <zmq_addon.hpp>

//#include "../quadlii/lib/common/monitor_t.h"
//#include "../quadlii/lib/common/ZmqMessage.h"

#include "common.h"
#include "bridge.h"

namespace asio = boost::asio;
namespace ip = asio::ip;

class tcp_session
  : public std::enable_shared_from_this<tcp_session>
{
public:
  
  //typedef asio::executor_work_guard<asio::io_context::executor_type> io_context_work;
  
  tcp_session( Bridge& bridge, ip::tcp::socket socket)
    : m_bridge( bridge ),
      m_socket( std::move( socket ) ), 
      m_transmitting( 0 )
      //m_zmqSocketRequest( m_zmqContext, zmq::socket_type::req ),
      //m_ioWork( asio::make_work_guard( m_ioContext ) ), // should this be in 'main' instead?
      //m_ioStrand( io_context )
      //m_ioThread( boost::bind( &asio::io_context::run, &io_context ) )
  { 
    std::cout << "session construct" << std::endl;
    //m_zmqSocketRequest.connect( "tcp://127.0.0.1:7411" );
  }
    
  virtual ~tcp_session() {
    //m_zmqSocketRequest.close();
    std::cout << "session destruct" << std::endl;
    
    //m_ioWork.reset();
    //m_ioThread.join();
    
  }

  void start();

private:

  enum { max_length = 65540 };  // total header and data for ipv4 is 65535

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
  std::mutex m_mutex;
  std::atomic<uint32_t> m_transmitting;
  
  qBuffers_t m_qBuffersAvailable;
  qBuffers_t m_qTxBuffersToBeWritten;
  vByte_t m_vTxInWrite;
  
  Bridge& m_bridge;
  
  //asio::io_context m_ioContext;
  //io_context_work m_ioWork;
  //boost::thread m_ioThread;
  //asio::io_context::strand m_ioStrand;
  
  //zmq::context_t m_zmqContext;
  //zmq::socket_t m_zmqSocketRequest;
  
  void ProcessPacket( uint8_t* pBegin, const uint8_t* pEnd );
  
  void GetAvailableBuffer( vByte_t& v );
  vByte_t GetAvailableBuffer();
  void QueueTxToWrite( vByte_t  );
  void LoadTxInWrite();
  void UnloadTxInWrite();
  
};


#endif /* TCP_SESSION_H */

