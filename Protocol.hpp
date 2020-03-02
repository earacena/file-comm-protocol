// Name: Emanuel Aracena Beriguete
// Date: February 14, 2020
// Filename: Protocol.hpp
// Description: Protocol for communication between single client/single server
//              transmission.

#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <memory>

#include "Logger.hpp"
#include "Common.hpp"
#include "Packet.hpp"

class Packet;

class Protocol {
public:
  Protocol();
  Protocol(const bool logging, const int mode);
  ~Protocol();

  void loop(const int socket);

  // Logging specific
  void error(const std::string & error_msg);
 
  // Allow logger to record protocol events
  //bool logging_;
  //Logger logger;

  // Used to differentiate multiple servers/clients on one physical computer
  // Not used for identity stage/layer
  std::string session_id;
  std::string receiver_id;
  // sequence numbers must be randomized to avoid collision with other connections
	int sequence_number;
  int sequence_number_y;

  // Protocol flags
  bool connection_initiated = false;
  bool asked_for_buffer = false;
  
  // Minimum buffers, min sizes, uniform for client/server until
  // they exchange comfortable upper limits
  char buffer[34] = {0};
  int min_buf_size = 34;
  int receiver_min_buf_size;
  // Each node must set this according, then create a buffer
};

#endif // PROTOCOL_HPP
