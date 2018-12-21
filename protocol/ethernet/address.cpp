/*
 * File:   mac.cpp
 * Author: sysadmin
 *
 * Created on October 5, 2017, 7:09 PM
 */

// https://en.wikipedia.org/wiki/Multicast_address

#include <stdexcept>

#define BOOST_SPIRIT_USE_PHOENIX_V3 1

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_rule.hpp>
#include <boost/spirit/include/qi_grammar.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_lit.hpp>

#include <boost/spirit/include/phoenix_operator.hpp>

#include "../../hexdump.h"

#include "address.h"

namespace {
  static const protocol::ethernet::address_t broadcast = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
  static const protocol::ethernet::address_t allzero   = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  static const protocol::ethernet::address_t user      = { 0x02, 0x00, 0x00, 0x00, 0x00, 0x00 };
  static const protocol::ethernet::address_t multicast = { 0x01, 0x00, 0x5e, 0x00, 0x00, 0x00 }; // IEEE 802 Multicast MAC Address (high 3 bytes)
}

namespace {

namespace qi = boost::spirit::qi;
//namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

//typedef uint8_t mac_t[ 6 ]; // has no operators
//typedef std::array<uint8_t,6> mac_t;  // has no insert
typedef std::vector<uint8_t> vMac_t;   // needs to be 6 == sizeof

template<typename Iterator>
struct mac_parser_t: qi::grammar<Iterator, vMac_t()> {

  mac_parser_t(): mac_parser_t::base_type( ruleBytes ) {

    ruleHexDigit = qi::char_( "0-9" )[qi::_val = qi::_1 - '0' ];
    ruleHexLc    = qi::char_( "a-f" )[qi::_val = qi::_1 - 'a' + 10 ];
    ruleHexUc    = qi::char_( "A-F" )[qi::_val = qi::_1 - 'A' + 10 ];
    ruleHex      = ruleHexDigit | ruleHexLc | ruleHexUc;
    ruleByte     = (ruleHex >> ruleHex)[qi::_val = (qi::_1 << 4) + qi::_2];
    ruleBytes    =
         ruleByte >> qi::lit(':')
      >> ruleByte >> qi::lit(':')
      >> ruleByte >> qi::lit(':')
      >> ruleByte >> qi::lit(':')
      >> ruleByte >> qi::lit(':')
      >> ruleByte
      ;
  }

  qi::rule<Iterator, uint8_t()> ruleHexDigit;
  qi::rule<Iterator, uint8_t()> ruleHexLc;
  qi::rule<Iterator, uint8_t()> ruleHexUc;
  qi::rule<Iterator, uint8_t()> ruleHex;
  qi::rule<Iterator, uint8_t()> ruleByte;
  qi::rule<Iterator, vMac_t()>  ruleBytes;

};
}

namespace protocol {
namespace ethernet {

std::ostream& operator<<( std::ostream& stream, const protocol::ethernet::address_t& mac ) {
  stream << HexDump<const uint8_t*>( mac, mac + 6, ':' );
  return stream;
}

///
/// \param macSrc
/// \param macDst
void ConvertStringToMac( const std::string& macSrc, address_t& macDst ) {

  vMac_t vResult;
  vResult.reserve( 6 );

  typedef std::string::const_iterator iterator_type;
  typedef mac_parser_t<iterator_type> mac_parser_string_t;
  mac_parser_string_t parser;

  std::string::const_iterator iter = macSrc.begin();
  std::string::const_iterator end = macSrc.end();
  auto result = qi::parse( iter, end, parser, vResult );

  if ( 1 != result ) throw std::runtime_error( "ill formed mac address (1)" );
  if ( iter != end ) throw std::runtime_error( "ill formed mac address (2)" );
  assert( sizeof( address_t ) == vResult.size() );

  std::memcpy( macDst, vResult.data(), sizeof( address_t ) );
}

bool address::IsMulticast( const address_t& mac ) {
  return 1 == ( mac[0] & 1 );
}

bool address::IsMulticast( const address& mac ) {
  return 1 == ( mac.m_mac[0] & 1 );
}

bool address::IsMulticast() const {
  return 1 == ( m_mac[0] & 1 );
}

bool address::IsAllZero( const address_t& mac ) {
  return 0 == std::memcmp( allzero, mac, sizeof( address_t ) );
}

bool address::IsAllZero( const address& mac ) {
  return IsAllZero( mac.m_mac );
}

bool address::IsAllZero() const {
  return 0 == std::memcmp( allzero, m_mac, sizeof( address_t ) );
}

bool address::IsBroadcast( const address_t& mac ) {
  return 0 == std::memcmp( mac, broadcast, 6 );
}

bool address::IsBroadcast( const address& mac ) {
  return IsBroadcast( mac.m_mac );
}

bool address::IsBroadcast() const {
  return 0 == std::memcmp( m_mac, broadcast, 6 );
}

address::address() {
  std::memset( m_mac, 0, sizeof( address_t ) );
}

address::address( const address_t& mac ) {
  std::memcpy( m_mac, mac, sizeof( address_t ) );
}

address::address( const std::string& sMac ) {
  ConvertStringToMac( sMac, m_mac );
}

address::address( const address& rhs ) {
  std::memcpy( m_mac, rhs.m_mac, sizeof( address_t ) );
}

const address_t& address::Value() const { return m_mac; }

const address& address::operator=( const address& rhs ) {
  std::memcpy( m_mac, rhs.m_mac, sizeof( address_t ) );
  return *this;
}

const address& address::operator=( const address_t& rhs ) {
  std::memcpy( m_mac, rhs, sizeof( address_t ) );
  return *this;
}

bool address::operator==( const address_t& rhs ) const {
  return 0 == std::memcmp( m_mac, rhs, sizeof( address_t ) );
}

bool address::operator==( const address& rhs ) const {
  return 0 == std::memcmp( m_mac, rhs.m_mac, sizeof( address_t ) );
}

std::ostream& address::Emit( std::ostream& stream ) const {
  stream << m_mac;
  return stream;
}

std::ostream& operator<<( std::ostream& stream, const address& mac ) {
  return mac.Emit( stream );
}
} // namespace ethernet
} // namespace protocol
