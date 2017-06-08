/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   datapath_id.cpp
 * Author: vagrant
 * 
 * Created on June 8, 2017, 8:06 AM
 */

#include "datapathid.h"

datapathid::datapathid( uint64_t datapathid ): m_datapathid( datapathid ) {
}

datapathid::~datapathid( ) {
}

std::ostream& operator<<(std::ostream& os, const datapathid& id ) {
  uint64_t id_( id.m_datapathid );
  static const char hex[] = "01234567890abcdef";
  for ( int ix = 0; ix < 2*sizeof(id_); ix++ ) {
    char c = hex[(id_ & 0xf000000000000000)>>60];
    os << c;
    id_ <<= 4;
  }
  return os;
}
