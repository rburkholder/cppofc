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
#include <functional>

#include <boost/asio/io_context.hpp>

#include "ovsdb_structures.h"

namespace asio = boost::asio;

namespace ovsdb {

class decode_impl;

class decode {
  friend class decode_impl;
public:

  decode(
    asio::io_context&,
    structures::f_t& f // will move the functions
    );
  virtual ~decode( );

protected:
private:

  typedef std::unique_ptr<decode_impl> pdecode_impl_t;
  pdecode_impl_t m_decode_impl;

  // TOOD: may keep a message queue of all messages (other than statistics)
  //   allows late-comers to obtain state transitions to current state
  //   provides message based mechanism for syncing, rather than locking the structure
  //  or run a strand for presenting updates, and requesting state and updates

  // TODO: these functions need to be assigned on construction
  //   allows them to be called with initial settings

  structures::f_t m_f;

};

} // namespace ovsdb

#endif /* OVSDB_H */

