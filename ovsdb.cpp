/*
 * File:   ovsdb.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net *
 *
 * Created on November 17, 2018, 11:25 AM
 */

#include <functional>

#include "ovsdb_impl.h"
#include "ovsdb.h"

namespace ovsdb {

decode::decode( asio::io_context& io_context,
  structures::f_t& f
  )
:
  m_f( f )
{
  m_decode_impl = std::make_unique<decode_impl>( std::ref( *this ), std::ref( io_context ) );
}

decode::~decode( ) {
}

} // namespace ovsdb