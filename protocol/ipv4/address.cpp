/*
 * File:   address.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on December 9, 2018, 4:54 PM
 */

#include <vector>
#include <cstring>

#define BOOST_SPIRIT_USE_PHOENIX_V3 1

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_rule.hpp>
#include <boost/spirit/include/qi_grammar.hpp>
//#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_lit.hpp>
#include <boost/spirit/include/qi_uint.hpp>

//#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

#include "address.h"

namespace {

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

typedef std::vector<uint8_t> vIpv4_t; // needs to be 4 == sizeof

// for an alternate version, see:
//   https://stackoverflow.com/questions/11307940/parsing-ipv4-addresses-using-boost-spirit

template<typename Iterator>
struct ipv4_parser_t: qi::grammar<Iterator, vIpv4_t()> {

  ipv4_parser_t(): ipv4_parser_t::base_type( ruleBytes ) {

    //ruleDigits = qi::ushort_;
    ruleDigits = qi::uint_parser<uint8_t,10,1,3>(); // https://www.boost.org/doc/libs/1_68_0/libs/spirit/doc/html/spirit/qi/reference/numeric/uint.html
    ruleBytes =
         ruleDigits >> qi::lit( '.' )
      >> ruleDigits >> qi::lit( '.' )
      >> ruleDigits >> qi::lit( '.' )
      >> ruleDigits;

  }

  qi::rule<Iterator, uint8_t()> ruleDigits;
  qi::rule<Iterator, vIpv4_t()> ruleBytes;

};

}

namespace protocol {
namespace ipv4 {

std::ostream& operator<<( std::ostream& stream, const protocol::ipv4::address_t& ipv4 ) {
  stream
    << (uint16_t)ipv4[0] << "."
    << (uint16_t)ipv4[1] << "."
    << (uint16_t)ipv4[2] << "."
    << (uint16_t)ipv4[3];
  return stream;
}

void ConvertStringToIPv4( const std::string& sIPv4, address_t& ipv4 ) {
  //typedef std::vector<uint8_t> vMac_t;
  vIpv4_t vResult;
  vResult.reserve( 4 );

  typedef std::string::const_iterator iterator_type;
  typedef ipv4_parser_t<iterator_type> ipv4_parser_string_t;
  ipv4_parser_string_t parser;

  std::string::const_iterator iter = sIPv4.begin();
  std::string::const_iterator end = sIPv4.end();
  auto result = qi::parse( iter, end, parser, vResult );

  if ( 1 != result ) throw std::runtime_error( "ill formed ipv4 address (1)" );
  if ( iter != end ) throw std::runtime_error( "ill formed ipv4 address (2)" );
  assert( sizeof( address_t ) == vResult.size() );

  std::memcpy( ipv4, vResult.data(), sizeof( address_t ) );
}

address::address( ) {
}

address::address( const address_t& ipv4 ) {
  std::memcpy( m_ipv4, ipv4, sizeof( address_t ) );
}

address::address( const address& rhs ) {
  std::memcpy( m_ipv4, rhs.m_ipv4, sizeof( address_t ) );
}

address::~address( ) {
}

bool address::operator==( const address_t& rhs ) const {
  return 0 == memcmp( m_ipv4, rhs, sizeof( address_t ) );
}

bool address::operator==( const address& rhs ) const {
  return 0 == memcmp( m_ipv4, rhs.m_ipv4, sizeof( address_t ) );
}

std::ostream& address::Emit( std::ostream& stream ) const {
  stream << m_ipv4;
  return stream;

}

std::ostream& operator<<( std::ostream& stream, const address& ipv4 ) {
  return ipv4.Emit( stream );
}

} // namespace ipv4
} // namespace protocol
