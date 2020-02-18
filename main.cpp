// Name: Emanuel Aracena Beriguete
// Date: February 11, 2020
// Filename: main.cpp
// Description: Simple TCP client/server.

#include <array>
#include <arpa/inet.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <ctime>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <unistd.h>

#include "Protocol.hpp"
#include "Logger.hpp"
#include "Server.hpp"
#include "Client.hpp"

void run_server (const bool logging, int port) {
  Server server(logging, port);
  server.initialize();
  server.run();
}


int run_client(const bool logging, const std::string & address, int port) {
  Client client(logging);
  int result = 0;
  result = client.connect_to_server(address, port);
  return result;
}

void print_help() {
  std::cout << "Proper usage: ./netnode [MODE] [ADDRESS] [PORT] [LOGGING]"  << std::endl;
  std::cout << "MODE: \n\tserver\n\tclient"  << std::endl;
  std::cout << "ADDRESS: \n\tclient - specify address to connect to\n\tserver - localhost" << std::endl;
  std::cout << "PORT: \n\tserver - specify port to listen";
  std::cout << "\n\tclient - specify server port to connect to" << std::endl;
  std::cout << "LOGGING: \n\t0 - disables logging";
  std::cout << "\n\t1 - enables logging" << std::endl;
}

int main(int argc, const char **argv) {

  // Check arguments for proper format  
  if (argc < 5) {
    print_help();
    return 0;
  }

  if (strcmp(argv[1], "server") != 0 && strcmp(argv[1], "client") != 0) {
    print_help();
    return 0;
  }

  if (strcmp(argv[4], "0") != 0 && strcmp(argv[4], "1") != 0) {
    print_help();
    return 0;
  }

  std::string mode(argv[1]);
  std::string address(argv[2]);
  int port = std::stoi(argv[3]);
  bool logging = (strcmp(argv[4], "0") == 0 ? false : true);
  int result = 0;
  if (mode == "server") 
    run_server(logging, port);
  else
    result = run_client(logging, address, port);

  return result;
}
