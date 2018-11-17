/* 
 * File:   ovsdb.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net *
 *
 * Created on November 17, 2018, 11:25 AM
 */

#ifndef OVSDB_H
#define OVSDB_H

#include <memory>

#include <boost/asio/io_context.hpp>

namespace asio = boost::asio;

class ovsdb_impl;

class ovsdb {
public:
  ovsdb( asio::io_context& io_context );
  virtual ~ovsdb( );
protected:
private:
  typedef std::shared_ptr<ovsdb_impl> povsdb_impl_t;
  povsdb_impl_t m_ovsdb_impl;
};

#endif /* OVSDB_H */

