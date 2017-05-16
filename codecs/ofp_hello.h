/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ofp_hello.h
 * Author: vagrant
 *
 * Created on May 16, 2017, 4:40 PM
 */

#ifndef OFP_HELLO_H
#define OFP_HELLO_H

#include <boost/endian/arithmetic.hpp>
using namespace boost::endian;
#include "../openflow/openflow-spec1.4.1.h"

namespace codec {

class ofp_hello_codec {
public:
  ofp_hello_codec( ofp_hello& packet );
  virtual ~ofp_hello_codec();
private:

};

}

#endif /* OFP_HELLO_H */

