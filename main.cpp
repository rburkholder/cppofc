/* 
 * File:   main.cpp
 * Author: Raymond Burkholder
 *         raymond@burkholder.net *
 *
 * Created on November 22, 2018, 4:17 PM
 */

#include <iostream>

#include "control.h"

int main( int argc, char** argv ) {
  
  int port( 6633 );

  if (argc != 2) {
    std::cout << "Usage: async_tcp_echo_server <port> (using " << port << ")\n";

  }
  else {
    port = std::atoi( argv[1] );
  }

  Control control( port );
  control.Start();

  return 0;
}

