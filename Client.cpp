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

  int min_client_buf_size = 50;
  int min_server_buf_size = 0;
  char client_buf[min_client_buf_size] = {0};
  

  int result = 0;
  std::string raw_packet;

  result = read(sock, client_buf, min_client_buf_size);
  raw_packet = client_buf;
  std::cout << "[+] Received packet [" << result << "]: " << raw_packet << std::endl;
  Packet packet;

  // Receive Buffer request 
  packet.parse(raw_packet);
  if (packet.type == "FF") {
    // Buffer size request
    Packet response;
    response = protocol_.craft_min_buffer_response_packet(min_client_buf_size);
    result = send(sock, response.encode().c_str(), strlen(response.encode().c_str()), 0);
    std::cout << "[+] Sent packet [" << result << "]: " << response.encode() << std::endl;


    // Find out server min buffer
    Packet request;
    request = protocol_.craft_min_buffer_request_packet();
    result = send(sock, request.encode().c_str(), strlen(request.encode().c_str()), 0);
    std::cout << "[+] Sent packet [" << result << "]: " << request.encode() << std::endl;

    result = read(sock, client_buf, min_client_buf_size);
    raw_packet = client_buf;
    response.parse(raw_packet);
    std::cout << "[+] Received packet [" << result << "]: " << raw_packet << std::endl;

    min_server_buf_size = protocol_.hex_to_dec(response.data);
  }


  // Handshake
  // Check message type

  result = read(sock, client_buf, min_client_buf_size);
  raw_packet = client_buf;
  packet.parse(raw_packet);

  int seq_num_y = 0;
  if (packet.type == "00") {
    // SYN packet, therefore this is the first communication being established

    // Check checksum
    if (packet.checksum != packet.compute_checksum()) {
      protocol_.error(std::string("ERROR [Client <- SYN] - Checksum not matching:") +
                      "...Expected: " + packet.checksum + "\n...Received: " + 
                      packet.compute_checksum());
      return -1;
    }
    

    // Extract sequence number x
    std::string seq_num_x_str(packet.data);
    int seq_num_x = protocol_.hex_to_dec(seq_num_x_str);
    Packet syn_ack_packet = protocol_.craft_syn_ack_packet(seq_num_x, packet);
    seq_num_y = protocol_.hex_to_dec(syn_ack_packet.data.substr(4,4));
    std::string raw_syn_ack_packet = syn_ack_packet.encode();
    result = send(sock, raw_syn_ack_packet.c_str(), strlen(raw_syn_ack_packet.c_str()), 0);
    std::cout << "[+] Sent packet [" << result << "]: " << raw_syn_ack_packet << std::endl;
  } 

  // Wait for ACK
  result = read(sock, client_buf, min_client_buf_size);
  std::string raw_response_packet = client_buf; 
  std::cout << "[+] Packet received [" << result << "]: " << raw_response_packet << std::endl;

  Packet response;
  response.parse(raw_response_packet);

  if (response.type == "02") {
    // ACK received, check session ids

    // Check checksum
    if (response.checksum != response.compute_checksum()) {
      protocol_.error(std::string("ERROR [Client <- ACK] - Checksum not matching:") +
                      "...Expected: " + response.checksum + "\n...Received: " + 
                      response.compute_checksum());
      return -1;
    }

    if (packet.receiver_id != response.receiver_id) {
      std::cout << "1" << std::endl;
      // terminate, incorrect recipient_id
      protocol_.error("ERROR - [Client <- ACK] - Incorrect receiver id, expected: " + 
                     packet.receiver_id + " received: " + response.receiver_id);
      return -1;
    }

    if (packet.sender_id != response.sender_id) {
      std::cout << "2" << std::endl;
      protocol_.error("ERROR - [Client <- ACK] - Incorrect sender id, expected: " + 
                     packet.sender_id + " received: " + response.sender_id);
      // terminate, incorrect session_id
      return -1;
    }

    if ((seq_num_y+1) != (protocol_.hex_to_dec(response.data.substr(0,4)))) {
      std::cout << "3: "<< seq_num_y << " " << response.data.substr(0,4) << std::endl;
      // terminate, incorrect increment for sequence number y
      protocol_.error(std::string("ERROR - [Client <- ACK] ") + 
                     "- Incorrect sequence number increment, expected: " + 
                      std::to_string(protocol_.hex_to_dec(packet.data) + 1) + " received: " +
                      std::to_string(protocol_.hex_to_dec(response.data.substr(0,4))));
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

