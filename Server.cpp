// nAme: Emanuel Aracena Beriguete
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

  std::cout << "[*] Server: listening..." << std::endl;
  std::cout << "[~]\n";
  // listen on the port
  if (listen(server_fd_, 3) < 0) {
    perror("listen");
    protocol_.error("Error - Listen system call failed.");
    exit(EXIT_FAILURE);
  }

  // Polling
  // ...

  int addrlen = sizeof(address_);


  // accept connection
  if ((sock_ = accept(server_fd_, (struct sockaddr *)&address_,
                            (socklen_t *)&addrlen)) < 0 ) {
    perror("accept");
    protocol_.error("Error - Accept system call failed.");
    exit(EXIT_FAILURE);
  }

  protocol_.connection_initiated = false;
  protocol_.asked_for_buffer = false;
  protocol_.receiver_min_buf_size = 34; 
  
  protocol_.ufds[0].fd = sock_;
  protocol_.ufds[0].events = POLLIN;

  while (true) {
    protocol_.loop(sock_); 
  }

  return;
}
