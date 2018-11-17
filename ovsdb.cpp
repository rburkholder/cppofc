/* 
 * File:   ovsdb.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net *
 * 
 * Created on November 17, 2018, 11:25 AM
 */

#include "ovsdb_impl.h"
#include "ovsdb.h"

ovsdb::ovsdb( asio::io_context& io_context ) {
  m_ovsdb_impl = povsdb_impl_t( new ovsdb_impl( io_context ) );
}

ovsdb::~ovsdb( ) {
}

