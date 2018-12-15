/*
 * File:   dns.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net *
 *
 * Created on December 14, 2018, 4:06 PM
 */

#ifndef DNS_H
#define DNS_H

#include <boost/endian/arithmetic.hpp>

namespace protocol {
namespace dns {

namespace endian = boost::endian;

enum rr_type {
  A = 1, // host address
  NS = 2, // authoritative name server
  MD = 3, // mail destination, obsolete, see MX
  MF = 4, // mail forwarder, obsolete, see MX
  CNAME = 5, // canonical name for an alias
  SOA = 6, // start of zone of authority
  MB = 7, // mailbox domain name, experimental
  MG = 8, // mail group memeber, experimental
  MR = 9, // mail rename domain name, experimental
  NULL_ = 10, // null RR, experimental
  WKS = 11, // well known service description
  PTR = 12, // domain name pointer
  HINFO = 13, // host information
  MINFO = 14, // mailbox or mail list information
  MX = 15, // mail exchange
  TXT = 16 // text strings
};

struct header_ { // https://tools.ietf.org/html/rfc1035
};

} // namespace dns
} // namespace protocol

#endif /* DNS_H */

