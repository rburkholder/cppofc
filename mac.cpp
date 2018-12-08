/*
 * File:   mac.cpp
 * Author: sysadmin
 *
 * Created on October 5, 2017, 7:09 PM
 */

// https://en.wikipedia.org/wiki/Multicast_address

#include <stdexcept>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_rule.hpp>
#include <boost/spirit/include/qi_grammar.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_lit.hpp>

//#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

#define BOOST_SPIRIT_USE_PHOENIX_V3 1

#include "mac.h"

namespace {
  static const mac_t broadcast = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
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

///
/// \param macSrc
/// \param macDst
void ConvertStringToMac( const std::string& macSrc, mac_t& macDst ) {
  typedef std::vector<uint8_t> vMac_t;
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
  assert( sizeof( mac_t ) == vResult.size() );

  std::memcpy( macDst, vResult.data(), sizeof( mac_t ) );
}

bool MacAddress::IsMulticast( const mac_t& mac ) {
  return 1 == ( mac[0] & 1 );
}

bool MacAddress::IsMulticast( const MacAddress& mac ) {
  return 1 == ( mac.m_mac[0] & 1 );
}

bool MacAddress::IsMulticast() const {
  return 1 == ( m_mac[0] & 1 );
}

bool MacAddress::IsBroadcast( const mac_t& mac ) {
  return 0 == std::memcmp( mac, broadcast, 6 );
}

bool MacAddress::IsBroadcast( const MacAddress& mac ) {
  return IsBroadcast( mac.m_mac );
}

bool MacAddress::IsBroadcast() const {
  return 0 == std::memcmp( m_mac, broadcast, 6 );
}

MacAddress::MacAddress() {
  std::memset( m_mac, 0, sizeof( mac_t ) );
}

MacAddress::MacAddress( const mac_t& mac ) {
  std::memcpy( m_mac, mac, sizeof( mac_t ) );
}

MacAddress::MacAddress( const std::string& sMac ) {
  ConvertStringToMac( sMac, m_mac );
}

MacAddress::MacAddress( const MacAddress& rhs ) {
  std::memcpy( m_mac, rhs.m_mac, sizeof( mac_t ) );
}

const mac_t& MacAddress::Value() const { return m_mac; }

bool MacAddress::operator==( const mac_t& rhs ) const {
  return 0 == std::memcmp( m_mac, rhs, sizeof( mac_t ) );
}

bool MacAddress::operator==( const MacAddress& rhs ) const {
  return 0 == std::memcmp( m_mac, rhs.m_mac, sizeof( mac_t ) );
}
