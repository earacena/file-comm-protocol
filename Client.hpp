// Name: Emanuel Aracena Beriguete
// Date: February 17, 2020
// Filename: Client.hpp
// Description: Client-side routines.

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <cstdio>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "Protocol.hpp"

class Client {

public:
  Client();
  Client(bool logging);
  void initialize();
  int connect_to_server(const std::string & address, int port);

private:
  Protocol protocol_;
  bool logging_;
};

#endif // CLIENT_HPP
