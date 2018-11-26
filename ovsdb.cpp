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

ovsdb::ovsdb( asio::io_context& io_context,
  structures::f_t& f
  )
:
  m_f( f )
{
  m_ovsdb_impl = std::make_unique<ovsdb_impl>( std::ref( *this ), std::ref( io_context ) );
}

ovsdb::~ovsdb( ) {
}

} // namespace ovsdb