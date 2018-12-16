/*
 * File:   datapath_id.h
 * Author: vagrant
 *
 * Created on June 8, 2017, 8:06 AM
 */

#ifndef DATAPATH_ID_H
#define DATAPATH_ID_H

#include <cstdint>
#include <ostream>

class datapathid {
  friend std::ostream& operator<<(std::ostream&, const datapathid& );
public:
  datapathid( uint64_t );
  virtual ~datapathid( );
private:
  uint64_t m_datapathid;
};

std::ostream& operator<<(std::ostream& os, const datapathid& id );

#endif /* DATAPATH_ID_H */

