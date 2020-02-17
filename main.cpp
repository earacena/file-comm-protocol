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

void run_server (const bool logging, int port) {
  std::cout << "[*] Running server mode..." << std::endl;
  Protocol protocol(logging, 1);

  int server_fd, new_socket, valread;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);
  char buffer[1024] = {0};

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
  address.sin_port = htons(port);

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

  // Send SYN packet
  int result = 0;
  std::string syn_packet = protocol.craft_syn_packet();

  int seq_num = protocol.hex_to_dec(syn_packet.substr(6));
  std::string session_id = syn_packet.substr(0,2);
  std::string recipient_id = syn_packet.substr(2,2);

  result = send(new_socket, syn_packet.c_str(), strlen(syn_packet.c_str()), 0);
  std::cout << "[+] Sent packet [" << result << "]: " << syn_packet << std::endl; 

  result = read(new_socket, buffer, 1024);
  std::string packet(buffer);
  std::cout << "[+] Received packet [" << result << "]: " << packet << std::endl;

  std::string packet_type(packet.substr(4,2));

  if (packet_type == "01") {
    // SYN-ACK received
    // Check session IDs
    if (packet.substr(0,2) != recipient_id) {
      // terminate, incorrect recipient_id
      protocol.error("Connection terminated - Incorrect recipient id, expected: " + recipient_id +
                     " received: " + packet.substr(0,2));
      return;
    }

    if (packet.substr(2,2) != session_id) {
      // terminate, incorrect session_id
      protocol.error("Connection terminated - Incorrect session id, expected: " + session_id +
                     " received: " + packet.substr(2,2));
      return;
    }

    // Check sequence number x increment 
    if (protocol.hex_to_dec(packet.substr(6,4)) != (seq_num + 1)) {
      // terminate, invalid sequence number increment
      protocol.error("Connection terminated - Incorrect sequence number increment, expected: " + 
                      std::to_string(seq_num+1) + " received: " +
                      std::to_string(protocol.hex_to_dec(packet.substr(6,4))));
      return;
    }

    int seq_num_y = 0;
    seq_num_y = protocol.hex_to_dec(packet.substr(10,4));
    std::string ack_packet(protocol.craft_ack_packet(seq_num_y, packet));

    // Send ACK packet
    result = send(new_socket, ack_packet.c_str(), strlen(ack_packet.c_str()), 0);
    std::cout << "[+] Sent packet [" << result << "]: " << ack_packet << std::endl;
    std::cout << "[+] Connection successfully established (Handshake complete)." << std::endl;
  }

// read message
//  valread = read(new_socket, buffer, 1024);
//  std::string read_message(buffer);
//  std::cout << "[+] Message received: " << read_message << std::endl;
//
// send message
//  std::cout << "[*] Sending message..." << std::endl;
//  send(new_socket, hello_message.c_str(), strlen(hello_message.c_str()), 0);
//  std::cout << "[+] Sent message: " << hello_message << std::endl;
  
  // terminate
}


// Client function(s)
int run_client(const bool logging, const std::string & address, int port) {
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
  serv_addr.sin_port = htons(port);

  // Convert IPv4/IPv6 addresses from text to binary form
  if (inet_pton(AF_INET, address.c_str(), &serv_addr.sin_addr) <= 0) {
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
  std::cout << "[+] Packet received ["  << valread << "]: "<< packet << std::endl;

  // Check message type
  std::string packet_type(packet.substr(4,2));

  std::string session_id = packet.substr(2,2);
  std::string recipient_id = packet.substr(0,2);
  int seq_num_y = 0;
  int result = 0;
  if (packet_type == "00") {
    // SYN packet, therefore this is the first communication being established
    // Extract sequence number x
    std::string seq_num_x_str(packet.substr(6));
    int seq_num_x = protocol.hex_to_dec(seq_num_x_str);
    std::string syn_ack_packet = protocol.craft_syn_ack_packet(seq_num_x, packet);
    seq_num_y = protocol.hex_to_dec(syn_ack_packet.substr(10, 4));
    result = send(sock, syn_ack_packet.c_str(), strlen(syn_ack_packet.c_str()), 0);
    std::cout << "[+] Sent message [" << result << "]: " << syn_ack_packet << std::endl;
  } 

  valread = read(sock, buffer, 1024);
  packet = std::string(buffer); 

  packet_type = packet.substr(4,2);
  if (packet_type == "02") {
    // ACK received, check session ids

    if (packet.substr(0,2) != recipient_id) {
      // terminate, incorrect recipient_id
      protocol.error("Connection terminated - Incorrect recipient id, expected: " + recipient_id +
                     " received: " + packet.substr(0,2));
      return -1;
    }

    if (packet.substr(2,2) != session_id) {
      protocol.error("Connection terminated - Incorrect session id, expected: " + session_id +
                     " received: " + packet.substr(2,2));
      // terminate, incorrect session_id
      return -1;
    }

    if (protocol.hex_to_dec(packet.substr(6,4)) != (seq_num_y + 1)) {
      // terminate, incorrect increment for sequence number y
      protocol.error("Connection terminated - Incorrect sequence number increment, expected: " + 
                      std::to_string(seq_num_y+1) + " received: " +
                      std::to_string(protocol.hex_to_dec(packet.substr(6,4))));
      return -1; 
    }
    
    std::cout << "[+] Connection successfully established (Handshake complete)." << std::endl;
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
