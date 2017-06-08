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

#include <boost/asio.hpp>

#include "codecs/common.h"

class tcp_session
  : public std::enable_shared_from_this<tcp_session>
{
public:
  tcp_session(boost::asio::ip::tcp::socket socket)
    : m_socket(std::move(socket)), m_transmitting( 0 )
  { 
      std::cout << "session construct" << std::endl;
  }
    
    virtual ~tcp_session() {
      std::cout << "session destruct" << std::endl;
    }

  void start();

private:

  enum { max_length = 17000 };

  void do_read();
  
  //void do_write(std::size_t length);

  // TODO:  need a write queue, need to update the xid value in the header
  //    so, create a method or class for handling queued messages and transactions
  //void do_write( vChar_t& v );
  void do_write();

  boost::asio::ip::tcp::socket m_socket;
  
  vChar_t m_vRx;
  //vChar_t m_vTx;
  
  typedef std::queue<vChar_t> qBuffers_t; 
  
  std::mutex m_mutex;
  std::atomic<uint32_t> m_transmitting;
  
  qBuffers_t m_qBuffersAvailable;
  qBuffers_t m_qTxBuffersToBeWritten;
  vChar_t m_vTxInWrite;
  
  void GetAvailableBuffer( vChar_t& v );
  vChar_t GetAvailableBuffer();
  void QueueTxToWrite( vChar_t  );
  void LoadTxInWrite();
  void UnloadTxInWrite();
  
};


#endif /* TCP_SESSION_H */
