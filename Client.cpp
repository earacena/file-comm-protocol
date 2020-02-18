// Name: Emanuel Aracena Beriguete
// Date: February 17, 2020
// Filename: Client.cpp
// Description: Client-side routine implementations.

#include "Client.hpp"

Client::Client() { 
  std::cout << "[+] Client instance created." << std::endl;
  logging_ = false;
  Protocol protocol(logging_, 1);
  protocol_ = protocol;
}

Client::Client(bool logging) { 
  logging_ = logging; 
  std::cout << "[+] Client instance created." << std::endl;
  Protocol protocol(logging_, 1);
  protocol_ = protocol;
}

void Client::initialize() {

}

int Client::connect_to_server(const std::string & address, int port) {
 
  int sock = 0;
  int valread;

  struct sockaddr_in serv_addr;
  std::string hello_message = "Hello from client";
  char buffer[1024] = {0};
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
    int seq_num_x = protocol_.hex_to_dec(seq_num_x_str);
    std::string syn_ack_packet = protocol_.craft_syn_ack_packet(seq_num_x, packet);
    seq_num_y = protocol_.hex_to_dec(syn_ack_packet.substr(10, 4));
    result = send(sock, syn_ack_packet.c_str(), strlen(syn_ack_packet.c_str()), 0);
    std::cout << "[+] Sent message [" << result << "]: " << syn_ack_packet << std::endl;
  } 

  // Wait for ACK
  valread = read(sock, buffer, 1024);
  packet = std::string(buffer); 

  packet_type = packet.substr(4,2);
  if (packet_type == "02") {
    // ACK received, check session ids

    if (packet.substr(0,2) != recipient_id) {
      // terminate, incorrect recipient_id
      protocol_.error("Connection terminated - Incorrect recipient id, expected: " + 
                     recipient_id + " received: " + packet.substr(0,2));
      return -1;
    }

    if (packet.substr(2,2) != session_id) {
      protocol_.error("Connection terminated - Incorrect session id, expected: " + 
                     session_id + " received: " + packet.substr(2,2));
      // terminate, incorrect session_id
      return -1;
    }

    if (protocol_.hex_to_dec(packet.substr(6,4)) != (seq_num_y + 1)) {
      // terminate, incorrect increment for sequence number y
      protocol_.error(std::string("Connection terminated ") + 
                     "- Incorrect sequence number increment, expected: " + 
                      std::to_string(seq_num_y+1) + " received: " +
                      std::to_string(protocol_.hex_to_dec(packet.substr(6,4))));
      return -1; 
    }
    
    std::cout << "[+] Connection successfully established (Handshake complete)." 
              << std::endl;
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

