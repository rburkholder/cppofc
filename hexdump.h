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

template<typename Stream, typename Iterator>
void HexDump( Stream& stream, Iterator begin, Iterator end ) {
  static const char hex[] = "0123456789abcdef";
  stream << "'";
  assert( begin < end );
  while ( begin != end ) {
    uint8_t c = *begin;
    char upper = hex[ c >> 4 ];
    char lower = hex[ c & 0x0f ];
    std::cout << upper << lower << " ";
    begin++;
    } // while
  stream << "'" << std::endl;
  }


#endif /* HEXDUMP_H */

