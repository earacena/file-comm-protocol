// Name: Emanuel Aracena Beriguete
// Date: February 17, 2020
// Filename: Server.hpp
// Description: Server-side routines.

#ifndef SERVER_HPP
#define SERVER_HPP

// Socket programming libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
 

#include "Protocol.hpp"
#include "Packet.hpp"


class Server {
public:
  Server(bool logging, int port);
  void initialize();
  void run();

private:
  Protocol protocol_;
  bool logging_;
  int port_;
  int server_fd_;
  int sock_;
  struct sockaddr_in address_;
};

#endif // SERVER_HPP
