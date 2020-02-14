//
// Name: Emanuel Aracena Beriguete
// Date: February 11, 2020
// Filename: main.cpp
// Description: Simple TCP client/server.

#include <arpa/inet.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <unistd.h>

// Server
#define SERVER_PORT 4444
#define BACKLOG 10

// Client
#define CLIENT_PORT 4444
#define MAX_DATA_SIZE 50

void run_server () {
  std::cout << "[*] Running server mode..." << std::endl;
}

void run_client() {
  std::cout << "[*] Running client mode..." << std::endl;
}

void print_help() {
  std::cout << "Proper usage: ./netnode [MODE]"  << std::endl;
  std::cout << "MODE: \n\tSERVER\n\tCLIENT"  << std::endl;
}

int main(int argc, char **argv) {

  // Check arguments for proper format  
  if (argc != 2) {
    print_help();
    return 0;
  }

  std::string mode(argv[1]);

  if (mode == "server")
    run_server();
  else
    run_client();


  return 0;
}
