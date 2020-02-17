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
  Protocol protocol(logging, 1);

  int server_fd, new_socket, valread;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);
  char buffer[1024] = {0};
  std::string hello_message = protocol.craft_syn_packet();

  // Create socket file descriptor
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket fail");
    protocol.error("Error - Socket system call failed.");
    exit(EXIT_FAILURE);
  }

  // Set socket options
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                    &opt, sizeof(opt))) {
    perror("setsockopt fail");
    protocol.error("Error - Setsockopt system call failed.");
    exit(EXIT_FAILURE);
  } 

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(SERVER_PORT);

  // Bind socket to port
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)  {
    perror("bind fail");
    protocol.error("Error - Bind system call failed.");
    exit(EXIT_FAILURE);
  } 

  // listen on the port
  if (listen(server_fd, 3) < 0) {
    perror("listen");
    protocol.error("Error - Listen system call failed.");
    exit(EXIT_FAILURE);
  }

  // accept connection
  if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                            (socklen_t *)&addrlen)) < 0 ) {
    perror("accept");
    protocol.error("Error - Accept system call failed.");
    exit(EXIT_FAILURE);
  }

  // read message
//  valread = read(new_socket, buffer, 1024);
//  std::string read_message(buffer);
//  std::cout << "[+] Message received: " << read_message << std::endl;
//
  // send message
  std::cout << "[*] Sending message..." << std::endl;
  send(new_socket, hello_message.c_str(), strlen(hello_message.c_str()), 0);
  std::cout << "[+] Sent message: " << hello_message << std::endl;
  
  // terminate
}


// Client function(s)
int run_client(const bool logging) {
  std::cout << "[*] Running client mode..." << std::endl;

  Protocol protocol(logging, 0);
 
  int sock = 0;
  int valread;

  struct sockaddr_in serv_addr;
  std::string hello_message = "Hello from client";
  char buffer[1024] = {0};
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    std::cout << "[-] Error: Socket creation error" << std::endl;
    protocol.error("Error - Socket creation failed.");
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(CLIENT_PORT);

  // Convert IPv4/IPv6 addresses from text to binary form
  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
    std::cout << "[-] Error: Invalid address / Address not supported" << std::endl;
    protocol.error("Error - Invalid address / Address not supported");
    return -1;
  }

  // Connect to server's socket
  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    std::cout << "[-] Error: Connection failed." << std::endl;
    protocol.error("Error - Connection failed (connect).");
    return -1;
  }

  // Receive message
  valread = read(sock, buffer, 1024);
  std::string packet(buffer);
  std::cout << "[+] Packet received: " << packet << std::endl;

  // Check message type
  std::string packet_type(packet.substr(4,2));

  std::cout << "[+] Packet type: " << packet_type << std::endl;
  if (packet_type == "00") {
    // SYN packet, therefore this is the first communication being established
    // Extract sequence number x
    std::string seq_num_x_str(packet.substr(6));
    int seq_num_x = protocol.hex_to_dec(seq_num_x_str);
    std::string syn_ack_packet = protocol.craft_syn_ack_packet(seq_num_x, packet);
    send(sock, packet.c_str(), strlen(packet.c_str()), 0);
  } 

  //  std::cout << "[*] Sending hello message..." << std::endl;
  //  send(sock, hello_message.c_str(), strlen(hello_message.c_str()), 0);
  //  
  //  std::cout << "[+] Message sent." << std::endl;
  //  valread = read(sock, buffer, 1024);
  //  std::string read_message(buffer);
  //  std::cout << "[+] Packet received: "<< read_message << std::endl;

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
