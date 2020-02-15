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

// Server
#define SERVER_PORT 4444

// Client
#define CLIENT_PORT 4444

void run_server (const bool logging) {
  std::cout << "[*] Running server mode..." << std::endl;
  Logger logger;
   
  if (logging) {
    std::cout << "[*] Initiating logger..." << std::endl;
    // Stamp log with timestamp
    std::time_t time = std::time(nullptr);
    std::string filename;
    filename = "./logs/";
    filename.append(std::to_string(static_cast<long int>(time)));
    filename = filename + "-server.txt";
    logger.set_filename(filename); 
  }

  int server_fd, new_socket, valread;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);
  char buffer[1024] = {0};
  std::string hello_message = "Hello from server";

  // Create socket file descriptor
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket fail");
    logger.record_event("Error - Socket system call failed.");
    logger.save_log();
    exit(EXIT_FAILURE);
  }

  // Set socket options
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                    &opt, sizeof(opt))) {
    perror("setsockopt fail");
    logger.record_event("Error - Setsockopt system call failed.");
    logger.save_log();
    exit(EXIT_FAILURE);
  } 

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(SERVER_PORT);

  // Bind socket to port
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)  {
    perror("bind fail");
    logger.record_event("Error - Bind system call failed.");
    logger.save_log();
    exit(EXIT_FAILURE);
  } 

  // listen on the port
  if (listen(server_fd, 3) < 0) {
    perror("listen");
    logger.record_event("Error - Listen system call failed.");
    logger.save_log();
    exit(EXIT_FAILURE);
  }

  // accept connection
  if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                            (socklen_t *)&addrlen)) < 0 ) {
    perror("accept");
    logger.record_event("Error - Accept system call failed.");
    logger.save_log();
    exit(EXIT_FAILURE);
  }

  // read message
  valread = read(new_socket, buffer, 1024);
  std::string read_message(buffer);
  logger.record_event("Message received: " + read_message);
  std::cout << "[+] Message received: " << read_message << std::endl;
  
  // send message
  send(new_socket, hello_message.c_str(), strlen(hello_message.c_str()), 0);
  logger.record_event("Message sent: " + hello_message);

  // save log
  logger.save_log();
  
  // terminate
}


// Client function(s)
int run_client(const bool logging) {
  std::cout << "[*] Running client mode..." << std::endl;

  Logger logger;
 
  if (logging) {
    // Stamp log with timestamp
    std::time_t time = std::time(nullptr);
    std::string filename;
    filename = "./logs/";
    filename.append(std::to_string(static_cast<long int>(time)));
    filename = filename + "-client.txt";
    logger.set_filename(filename); 
  }

 
  int sock = 0;
  int valread;

  struct sockaddr_in serv_addr;
  std::string hello_message = "Hello from client";
  char buffer[1024] = {0};
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    std::cout << "[-] Error: Socket creation error" << std::endl;
    logger.record_event("Error - Socket creation failed.");
    logger.save_log();
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(CLIENT_PORT);

  // Convert IPv4/IPv6 addresses from text to binary form
  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
    std::cout << "[-] Error: Invalid address / Address not supported" << std::endl;
    logger.record_event("Error - Invalid address / Address not supported");
    logger.save_log();
    return -1;
  }

  // Connect to server's socket
  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    std::cout << "[-] Error: Connection failed." << std::endl;
    logger.record_event("Error - Connection failed (connect).");
    logger.save_log();
    return -1;
  }

  // send message
  std::cout << "[*] Sending hello message..." << std::endl;
  send(sock, hello_message.c_str(), strlen(hello_message.c_str()), 0);
  logger.record_event("Sending message: " + hello_message);
  
  std::cout << "[+] Hello message sent." << std::endl;
  valread = read(sock, buffer, 1024);
  std::string read_message(buffer);
  std::cout << "[+] Message received: "<< read_message << std::endl;
  logger.record_event("Message received: " + read_message);

  logger.save_log();
  return 0;
}

void print_help() {
  std::cout << "Proper usage: ./netnode [MODE] [LOGGING]"  << std::endl;
  std::cout << "MODE: \n\tserver\n\tclient"  << std::endl;
  std::cout << "LOGGING: \n\t0 - disables logging";
  std::cout << "\n\t1 - enables logging" << std::endl;
}

int main(int argc, const char **argv) {

  // Check arguments for proper format  
  if (argc < 3) {
    print_help();
    return 0;
  }

  if (strcmp(argv[1], "server") != 0 && strcmp(argv[1], "client") != 0) {
    print_help();
    return 0;
  }

  if (strcmp(argv[2], "0") != 0 && strcmp(argv[2], "1") != 0) {
    print_help();
    return 0;
  }

  std::string mode(argv[1]);
  bool logging = (strcmp(argv[2], "0") == 0 ? false : true);
  int result = 0;
  if (mode == "server")
    run_server(logging);
  else
    result = run_client(logging);

  return result;
}
