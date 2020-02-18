// Name: Emanuel Aracena Beriguete
// Date: February 17, 2020
// Filename: Server.cpp
// Description: Server-side routine implementations.

#include "Server.hpp"

Server::Server(bool logging, int port) {
  std::cout << "[+] Server instance created. " << std::endl;
  logging_ = logging;
  port_ = port;
}

void Server::initialize() {
  std::cout << "[*] Server: initializing..." << std::endl;
  Protocol protocol(logging_, port_);
  protocol_ = protocol;
  
  int valread = 0;
  int opt = 1;
  int addrlen = sizeof(address_);
  char buffer[1024] = {0};

  // Create socket file descriptor
  if ((server_fd_ = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket fail");
    protocol.error("Error - Socket system call failed.");
    exit(EXIT_FAILURE);
  }

  // Set socket options
  if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                    &opt, sizeof(opt))) {
    perror("setsockopt fail");
    protocol.error("Error - Setsockopt system call failed.");
    exit(EXIT_FAILURE);
  } 

  address_.sin_family = AF_INET;
  address_.sin_addr.s_addr = INADDR_ANY;
  address_.sin_port = htons(port_);

  // Bind socket to port
  if (bind(server_fd_, (struct sockaddr *)&address_, sizeof(address_)) < 0)  {
    perror("bind fail");
    protocol.error("Error - Bind system call failed.");
    exit(EXIT_FAILURE);
  } 
}

void Server::run() {
  std::cout << "[*] Server: running..." << std::endl;
  int addrlen = sizeof(address_);
  char buffer[1024] = {0};

  // listen on the port
  if (listen(server_fd_, 3) < 0) {
    perror("listen");
    protocol_.error("Error - Listen system call failed.");
    exit(EXIT_FAILURE);
  }

  // accept connection
  if ((sock_ = accept(server_fd_, (struct sockaddr *)&address_,
                            (socklen_t *)&addrlen)) < 0 ) {
    perror("accept");
    protocol_.error("Error - Accept system call failed.");
    exit(EXIT_FAILURE);
  }

  // Send SYN packet
  int result = 0;
  std::string syn_packet = protocol_.craft_syn_packet();

  int seq_num = protocol_.hex_to_dec(syn_packet.substr(6));
  std::string session_id = syn_packet.substr(0,2);
  std::string recipient_id = syn_packet.substr(2,2);

  result = send(sock_, syn_packet.c_str(), strlen(syn_packet.c_str()), 0);
  std::cout << "[+] Sent packet [" << result << "]: " << syn_packet << std::endl; 

  result = read(sock_, buffer, 1024);
  std::string packet(buffer);
  std::cout << "[+] Received packet [" << result << "]: " << packet << std::endl;

  std::string packet_type(packet.substr(4,2));

  if (packet_type == "01") {
    // SYN-ACK received
    // Check session IDs
    if (packet.substr(0,2) != recipient_id) {
      // terminate, incorrect recipient_id
      protocol_.error("Connection terminated - Incorrect recipient id, expected: " + 
                      recipient_id + " received: " + packet.substr(0,2));
      return;
    }

    if (packet.substr(2,2) != session_id) {
      // terminate, incorrect session_id
      protocol_.error("Connection terminated - Incorrect session id, expected: " + 
                      session_id + " received: " + packet.substr(2,2));
      return;
    }

    // Check sequence number x increment 
    if (protocol_.hex_to_dec(packet.substr(6,4)) != (seq_num + 1)) {
      // terminate, invalid sequence number increment
      protocol_.error(std::string("Connection terminated -") + 
                      " Incorrect sequence number increment, expected: " + 
                      std::to_string(seq_num+1) + " received: " +
                      std::to_string(protocol_.hex_to_dec(packet.substr(6,4))));
      return;
    }

    int seq_num_y = 0;
    seq_num_y = protocol_.hex_to_dec(packet.substr(10,4));
    std::string ack_packet(protocol_.craft_ack_packet(seq_num_y, packet));

    // Send ACK packet
    result = send(sock_, ack_packet.c_str(), strlen(ack_packet.c_str()), 0);
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
  return;

}
