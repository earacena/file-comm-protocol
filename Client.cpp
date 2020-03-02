// Name: Emanuel Aracena Beriguete
// Date: February 17, 2020
// Filename: Client.cpp
// Description: Client-side routine implementations.

#include "Client.hpp"

Client::Client() { 
  std::cout << "[+] Client instance created." << std::endl;
  logging_ = false;
  Protocol protocol(logging_, 0);
  protocol_ = protocol;
}

Client::Client(bool logging) { 
  logging_ = logging; 
  std::cout << "[+] Client instance created." << std::endl;
  Protocol protocol(logging_, 0);
  protocol_ = protocol;
}

void Client::initialize() {

}

int Client::connect_to_server(const std::string & address, int port) {
 
  int sock = 0;

  struct sockaddr_in serv_addr;
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    std::cout << "[-] Error: Socket creation error" << std::endl;
    protocol_.error("Error - Socket creation failed.");
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);

  // Convert IPv4/IPv6 addresses from text to binary form
  if (inet_pton(AF_INET, address.c_str(), &serv_addr.sin_addr) <= 0) {
    std::cout << "[-] Error: Invalid address / Address not supported" << std::endl;
    protocol_.error("Error - Invalid address / Address not supported");
    return -1;
  }

  // Connect to server's socket
  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    std::cout << "[-] Error: Connection failed." << std::endl;
    protocol_.error("Error - Connection failed (connect).");
    return -1;
  }

  int result = 0;
  std::string raw_packet;

  protocol_.connection_initiated = true;
  protocol_.asked_for_buffer = false;
  protocol_.receiver_min_buf_size = 34;
  // Connection loop
  while (true) {
    protocol_.loop(sock);
  }

  return 0;
}

