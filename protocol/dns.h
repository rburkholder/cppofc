/*
 * File:   dns.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net *
 *
 * Created on December 14, 2018, 4:06 PM
 */

#ifndef DNS_H
#define DNS_H

#include <string>

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

enum rr_qtype { // includes all rr_type values
  AXFR = 252, // request for at transfer of an entire zone
  MAILB = 253, // request for mailbox-related records (MB, MG, or MR)
  MAILA = 254, // request for all mail agent RRs (obsolete - see MX)
  ALL = 255 // request for all records (NOTE: seen as a '*')
};

enum rr_class {
  IN = 1, // the internet
  CS = 2, // CSNET (obsolete)
  CH = 3, // CHAOS class
  HS = 4  // Hesiod [dyer 87]
};

enum rr_qclass {
  ANY = 255 // any class (NOTE: seen as a '*'), superset of rr_class
};

/*
 * general RR (resource record) layout:
 *   NAME
 *   TYPE
 *   CLASS
 *   TTL
 *   RDLENGTH
 *   RDATA
 *
 * standard RR: NS, SOA, CNAME, and PTR
 */

/*
 * notes:
 * name server logic in [RFC-1034]  https://tools.ietf.org/html/rfc1034
 */

struct soa_ { // measurements in seconds
  endian::big_uint32_t serial; // version number of the original copy of the zone, wraps using sequence logic
  endian::big_uint32_t refresh; // time interval before the zone should be refreshed
  endian::big_uint32_t retry;  // time interval that should elapse before a failed refresh should be retried
  endian::big_uint32_t expire; // time value that specifies the upper limit on the time interval that can elapse before the zone is no longer authoritative
  endian::big_uint32_t minimum; // minimum TTL field that should be exported with any RR from this zone
};

enum header_flag {
  header_flag_qr    = 0x8000, // 0 query, 1 response
  header_flag_opcode= 0x7100, // 0 query, 1 inverse query, 2 server status, 3 - 15 reserved
  header_flag_aa    = 0x0400, // authoritative answer
  header_flag_tc    = 0x0200, // truncation
  header_flag_rd    = 0x0100, // recursion desired
  header_flag_ra    = 0x0080, // recursion available
  header_flag_z     = 0x0070, // future, zeros
  header_flag_rcode = 0x000f  // rcode:
};

enum rcode {
  no_error = 0,
  format_error = 1,
  server_failure = 2,
  name_error = 3,
  not_implemented = 4,
  refused = 5
    // 6 - 15 reserved for future use
};

/*
 * message format:
 *   header
 *   question (question for the name server), QNAME, QTYPE, QCLASS
 *   answer (RRs answering the question), NAME, TYPE, CLASS, TTL, RDLENGTH, RDATA
 *   authority (RRs pointing toward an authority), NAME, TYPE, CLASS, TTL, RDLENGTH, RDATA
 *   additional (RRs hoding additional information), NAME, TYPE, CLASS, TTL, RDLENGTH, RDATA
 */

struct header_ { // https://tools.ietf.org/html/rfc1035
  endian::big_uint16_t id;  // copied to reply from request
  endian::big_uint16_t flags;
  endian::big_uint16_t qdcount; // cnt in question section, rfc1035, page 27
  endian::big_uint16_t ancount; // cnt in answer section
  endian::big_uint16_t nscount; // cnt in name server resource records section
  endian::big_uint16_t arcount; // cnt in additional records section
};

struct question_ {
  std::string m_name;
  uint16_t m_qtype;
  uint16_t m_qclass;
};

struct rr_ {
  std::string m_name;
  uint16_t m_type;
  uint16_t m_class;
  uint32_t m_ttl;
  uint16_t m_rdlen;
  uint8_t* m_prd;
};

// ** Header

class Header {
  friend std::ostream& operator<<( std::ostream&, const Header& );
public:

  Header( header_& );
  virtual ~Header();

protected:
private:
  header_& m_header;

  std::ostream& Emit( std::ostream& stream ) const;
};

std::ostream& operator<<( std::ostream& stream, const Header& header );

// ** Packet

class Packet {
  friend std::ostream& operator<<( std::ostream&, const Packet& );
public:

  Packet( uint8_t& ); // TODO: require packet length for validation purposes
  virtual ~Packet();

  const header_& GetHeader() {
    return *m_pHeader;
  }

protected:
private:

  uint8_t* m_pPacket;
  header_* m_pHeader;
  uint8_t* m_pData;
  //Content m_Content;

  uint8_t* DecodeName( uint8_t* p, std::string& name ) const;
  uint8_t* DecodeQuestion( uint8_t* p, question_& q ) const;
  uint8_t* DecodeResourceRecord( uint8_t* p, rr_& ) const;

};

} // namespace dns
} // namespace protocol

#endif /* DNS_H */

