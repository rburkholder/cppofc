/*
 * File:   Buffer.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on December 1, 2018, 10:48 PM
 */

#include "Buffer.h"

//Buffer::Buffer( asio::io_context::strand& strand )
//: m_strandBufferOps( strand )
//{
//}

Buffer::Buffer() {}

Buffer::~Buffer() {}

vByte_t Buffer::ObtainBuffer() {
  std::unique_lock<std::mutex> lock( m_mutex );
  vByte_t vByte = std::move( m_qTxBuffersToBeWritten.front() );
  m_qTxBuffersToBeWritten.pop();
  return vByte;
}

void Buffer::ReturnBuffer( vByte_t& vByte) {
  std::unique_lock<std::mutex> lock( m_mutex );
  m_qBuffersAvailable.push( std::move( vByte ) );
}
