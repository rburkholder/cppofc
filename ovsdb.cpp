/* 
 * File:   ovsdb.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net *
 * 
 * Created on November 17, 2018, 11:25 AM
 */

#include "ovsdb_impl.h"
#include "ovsdb.h"

ovsdb::ovsdb( asio::io_context& io_context,
  fSwitchUpdate_t fSwitchUpdate,
  fPortUpdate_t fPortUpdate,
  fInterfaceUpdate_t fInterfaceUpdate,
  fStatisticsUpdate_t fStatisticsUpdate
  ) 
: 
  m_fSwitchUpdate( fSwitchUpdate ),
  m_fPortUpdate( fPortUpdate ),
  m_fInterfaceUpdate( fInterfaceUpdate ),
  m_fStatisticsUpdate( fStatisticsUpdate )
{
  m_ovsdb_impl = povsdb_impl_t( new ovsdb_impl( *this, io_context ) );
}

ovsdb::~ovsdb( ) {
}

