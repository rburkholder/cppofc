/* 
 * File:   hexdump.h
 * Author: Raymond Burkholder
 *         raymond@burkholder.net
 *
 * Created on June 12, 2017, 1:06 PM
 */

#ifndef HEXDUMP_H
#define HEXDUMP_H

#include <cassert>
#include <cstdint>
#include <iostream>

// https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Making_New_Friends

template<typename Iterator> class HexDump;

template<typename Iterator>
std::ostream& operator<<( std::ostream& ost, const HexDump<Iterator>& );

template<typename Iterator>
class HexDump {
  friend std::ostream& operator<< <>( std::ostream&, const HexDump<Iterator>& );
public:
  
  HexDump( const Iterator begin, const Iterator end, char separator = ' ' )
    : m_begin( begin ), m_end( end ), m_separator( separator )  { 
      assert( begin <= end );
    };
 
  std::ostream& Emit( std::ostream& stream ) const {
  
    static const char hex[] = "0123456789abcdef";

    // http://www.cplusplus.com/forum/windows/51591/
    std::ios_base::fmtflags oldFlags = std::cout.flags();
    std::streamsize         oldPrec  = std::cout.precision();
    char                    oldFill  = std::cout.fill();
    
    Iterator begin( m_begin );

    stream << "'";
    bool bStarted( false );
    
    while ( begin != m_end ) {

      if (bStarted) stream << m_separator;
      else bStarted = true;

      uint8_t c = *begin;
      char upper = hex[ c >> 4 ];
      char lower = hex[ c & 0x0f ];
      stream << upper << lower;
      begin++;
      } // while
    
    stream << "'";

    std::cout.flags(oldFlags);
    std::cout.precision(oldPrec);
    std::cout.fill(oldFill);

    return stream;
  }

private:
  const Iterator m_begin;
  const Iterator m_end;
  const char m_separator;
};
 
template<typename Iterator>
std::ostream& operator<<( std::ostream& ost, const HexDump<Iterator>& hexdump ) {
  hexdump.Emit( ost );
  return ost;
}
 
#endif /* HEXDUMP_H */

