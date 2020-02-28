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

  // Get buffer size
  // Set Server min buffer size
  int server_min_buf_size = 40;
  int client_min_buf_size = 0;
  int result = 0;
  char server_buf[server_min_buf_size] = {0};

  BufferRequestPacket buffer_request_server;
  std::string payload = "";
  buffer_request_server.craft(protocol_, payload);
  std::string raw_packet = buffer_request_server.encode();
  
  // Send min buffer request
  result = send(sock_, raw_packet.c_str(), strlen(raw_packet.c_str()), 0);
  std::cout << "[+] Sent packet [" << result << "]: " << raw_packet << std::endl;

  // Read response
  result = read(sock_, server_buf, server_min_buf_size); 
  BufferResponsePacket buffer_response_server;
  raw_packet = server_buf;
  buffer_response_server.parse(raw_packet);
  std::cout << "[+] Received packet [" << result << "]: " << raw_packet << std::endl;
 
  // Set new client min buffer size
  client_min_buf_size = hex_to_dec(buffer_response_server.data); 

  // Wait for buffer request from client
  result = read(sock_, server_buf, server_min_buf_size);
  BufferRequestPacket buffer_request_client;
  raw_packet = server_buf;
  request.parse(raw_packet);
  std::cout << "[+] Received packet [" << result << "]:" << raw_packet << std::endl; 
  
  if (request.type == "FF") {
    // Buffer request received
    BufferResponsePacket buffresponse = protocol_.craft_min_buffer_response_packet(server_min_buf_size);
    raw_packet = response.encode();
    result = send(sock_, raw_packet.c_str(), strlen(raw_packet.c_str()), 0);
    std::cout << "[+] Sent packet [" << result << "]: " << raw_packet << std::endl;
  }

  // Handshake
  // Send SYN packet
  Packet syn_packet = protocol_.craft_syn_packet();
  int seq_num = protocol_.hex_to_dec(syn_packet.data);

  std::string raw_syn_packet = syn_packet.encode();

  result = send(sock_, raw_syn_packet.c_str(), client_min_buf_size, 0);
  std::cout << "[+] Sent packet [" << result << "]: " << raw_syn_packet << std::endl; 

  result = read(sock_, server_buf, server_min_buf_size);
  raw_packet = server_buf;
  Packet syn_ack_packet;
  syn_ack_packet.parse(raw_packet);

  std::cout << "[+] Received packet [" << result << "]: " << raw_packet << std::endl;

  if (syn_ack_packet.type == "01") {
    // SYN-ACK received

    // Check checksums
    if (syn_ack_packet.checksum != syn_ack_packet.compute_checksum()) {
      protocol_.error(std::string("ERROR [Server <- SYN-ACK] - Checksum not matching:") +
                      "...Expected: " + syn_ack_packet.checksum + "\n...Received: " +
                      syn_ack_packet.compute_checksum());
      return;
    }


    // Check session IDs
    if (syn_ack_packet.receiver_id != protocol_.session_id_) {
      // terminate, incorrect receiver_id
      protocol_.error("ERROR [Server <- SYN-ACK] - Incorrect receiver id, expected: " + 
                      protocol_.session_id_ + " received: " + syn_ack_packet.receiver_id);
      return;
    }

    if (syn_ack_packet.sender_id != syn_packet.receiver_id) {
      // terminate, incorrect session_id
      protocol_.error("ERROR [Server <- SYN_ACK] - Incorrect sender id, expected: " + 
                      syn_packet.receiver_id + " received: " + syn_ack_packet.sender_id);
      return;
    }

    // Check sequence number x increment 
    if (protocol_.hex_to_dec(syn_packet.data) + 1 != 
        (protocol_.hex_to_dec(syn_ack_packet.data.substr(0,4)))) {
      // terminate, invalid sequence number increment
      protocol_.error(std::string("ERROR [Server <- SYN_ACK] -") + 
                      " Incorrect sequence number increment, expected: " + 
                      std::to_string(protocol_.hex_to_dec(syn_ack_packet.data.substr(0,4)))+ 
                      " received: " +
                      std::to_string(protocol_.hex_to_dec(syn_packet.data) + 1));
      return;
    }

    int seq_num_y = 0;
    seq_num_y = protocol_.hex_to_dec(syn_ack_packet.data.substr(4,4));
    Packet ack_packet(protocol_.craft_ack_packet(seq_num_y, syn_ack_packet));

    std::string raw_ack_packet = ack_packet.encode();

    // Send ACK packet
    result = send(sock_, raw_ack_packet.c_str(), client_min_buf_size, 0);
    std::cout << "[+] Sent packet [" << result << "]: " << raw_ack_packet << std::endl;
    std::cout << "[+] Connection successfully established (Handshake complete)." << std::endl;
  }

  // Use case: Simulated buffer overflow
  std::string message = std::string("This is a very long message, longer than the size") +
                                    "of the buffer that is supposed to receive it. This " + 
                                    "is created to test the exachange of buffer sizes " + 
                                    "between a server and client.";  

  Packet buffer_test_packet = protocol_.craft_data_packet(message, syn_ack_packet.sender_id);
  std::cout << "[*] Packing message in packet: \"" << message << "\"" << std::endl;
  raw_packet = buffer_test_packet.encode();
  result = send(sock_, raw_packet.c_str(), (size_t)client_min_buf_size, 0);  
  std::cout << "[+] Sent packet [" << result << "]: " << raw_packet << std::endl;



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
