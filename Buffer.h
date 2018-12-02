/*
 * File:   Buffer.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on December 1, 2018, 10:48 PM
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <queue>
#include <mutex>

//#include <boost/asio/io_context.hpp>
//#include <boost/asio/strand.hpp>

#include "common.h"

// TODO: make use of strand to obtain and return buffers
//       use flags to ensure xor(strand, mutex) use
// TODO: can functions be defined which accept/disseminate buffers and a follow on function?
//    use template?  Issue is that it needs to be quick in, quick out.  So supplied
//    function will need to be run in a generic or self-assigned thread
//      so, optional third parameter is a strand or io_context in which to run?

//namespace asio = boost::asio;

class Buffer {
public:

  Buffer();
  //Buffer( asio::io_context::strand& strand );
  virtual ~Buffer();

  vByte_t ObtainBuffer();
  void ReturnBuffer( vByte_t& );

  //asio::io_context::strand& Strand() { return m_strandBufferOps; }

protected:
private:

  //asio::io_context::strand m_strandBufferOps;

  std::mutex m_mutex;

  typedef std::queue<vByte_t> qBuffers_t;

  qBuffers_t m_qBuffersAvailable;
  qBuffers_t m_qTxBuffersToBeWritten;

  Buffer( const Buffer& ) = delete;

};

#endif /* BUFFER_H */

