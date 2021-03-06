/*
 * File:   Buffer.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on December 1, 2018, 10:48 PM
 */

#include <cassert>

#include "Buffer.h"

//Buffer::Buffer( asio::io_context::strand& strand )
//: m_strandBufferOps( strand )
//{
//}

Buffer::Buffer() {}

Buffer::~Buffer() {}

vByte_t Buffer::ObtainBuffer() {
  //std::unique_lock<std::mutex> lock( m_mutex );
  if ( m_qBuffersAvailable.empty() ) {
    vByte_t v;
    v.reserve( 66000 );
    m_qBuffersAvailable.push( std::move( v ) );
  }
  vByte_t vByte = std::move( m_qBuffersAvailable.front() );
  m_qBuffersAvailable.pop();
  return vByte;
}

void Buffer::AddBuffer( vByte_t& vByte) {
  //std::unique_lock<std::mutex> lock( m_mutex );
  //vByte.clear(); // don't do this as this is used for queued storage
  assert( 66000 <= vByte.capacity() );
  m_qBuffersAvailable.push( std::move( vByte ) );
}

bool Buffer::Empty() {
  //std::unique_lock<std::mutex> lock( m_mutex );
  return m_qBuffersAvailable.empty();
}

const vByte_t& Buffer::Front() const {
  return m_qBuffersAvailable.front();
}