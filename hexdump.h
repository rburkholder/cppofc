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

template<typename Stream, typename Iterator>
Stream& HexDump( Stream& stream, Iterator begin, Iterator end, char sep = ' ' ) {
  static const char hex[] = "0123456789abcdef";
  
  // http://www.cplusplus.com/forum/windows/51591/
  std::ios_base::fmtflags oldFlags = std::cout.flags();
  std::streamsize         oldPrec  = std::cout.precision();
       char               oldFill  = std::cout.fill();

  stream << "'";
  assert( begin < end );
  bool bStarted( false );
  while ( begin != end ) {
    
    if (bStarted) stream << sep;
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


#endif /* HEXDUMP_H */

