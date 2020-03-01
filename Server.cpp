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

  // Get buffer size
  // Set Server min buffer size
  int server_min_buf_size = 40;
  int client_min_buf_size = 0;
  int result = 0;
  char server_buf[server_min_buf_size] = {0};

  std::cout << "[*] Server: running..." << std::endl;

  // listen on the port
  if (listen(server_fd_, 3) < 0) {
    perror("listen");
    protocol_.error("Error - Listen system call failed.");
    exit(EXIT_FAILURE);
  }

  bool buffers_exchanged = false;
  bool handshake_complete = false;


  // Polling
  // ...

  while (true) {
    int addrlen = sizeof(address_);
    std::string raw_packet = "";

    // accept connection
    if ((sock_ = accept(server_fd_, (struct sockaddr *)&address_,
                              (socklen_t *)&addrlen)) < 0 ) {
      perror("accept");
      protocol_.error("Error - Accept system call failed.");
      exit(EXIT_FAILURE);
    }

    //-------------------------------------------------------------
    // SEND FIRST, Server initiates connection
    //-------------------------------------------------------------
    
    // Normally a SYN packet would be only SEND FIRST action on first loop
    // but since server is passively listening, client will be sending SYN
    // most of the time, unless current protocol changes
    

    //-------------------------------------------------------------
    // RECEIVE FIRST,  Server receives requests/packets first 
    //-------------------------------------------------------------

    // retrieve packet
    int result = 0;
    result = recv(sock_, server_buf, server_min_buf_size, 0);
    if (result < 0) {
      // Error received
      std::cout << "[-] Error ~> Server : recv()" << std::endl;
      return;
    } else if (result == 0) {
      // remote client closed connection
      std::cout << "[-] Error ~> Server : Remote client closed connection." << std::endl;
      return;
    } else {
      // result is set to number of transmitted bytes
      raw_packet = server_buf;
      std::cout << "[+] Received packet [" << result << "]: " << raw_packet << std::endl;
      
      // Extract raw packet type to determine how it's parsed
      std::string packet_type = raw_packet.substr(20,2);

      // Upcast to specific packet object
      std::unique_ptr<Packet> packet_ptr;
      if (packet_type == "00") {
        // SYN packet
        packet_ptr = std::make_unique<SynPacket>();
      } else if (packet_type == "01") {
        // SYN-ACK packet
        packet_ptr = std::make_unique<SynAckPacket>();
      } else if (packet_type == "02") {
        // ACK packet
        packet_ptr = std::make_unique<AckPacket>();
      } else if (packet_type == "07") {
        // Buffer Request packet
        packet_ptr = std::make_unique<BufferRequestPacket>();
      } else if (packet_type == "08") {
        // Buffer Response packet
        packet_ptr = std::make_unique<BufferResponsePacket>();
      } else if (packet_type == "09") {
        // General Data packet
        packet_ptr = std::make_unique<DataPacket>();
      } else {
        std::cout << "[-] Error ~> Server : Invalid packet type [" << packet_type << "]" << std::endl;
        break;
      }
      // Parse into appropirate object
      packet_ptr->parse(raw_packet);

      // Verify checksum
      if (packet_ptr->checksum != packet_ptr->compute_checksum()) {
        std::cout << "[-] Error ~> Server : Invalid Checksum, Exp: " << packet_ptr->checksum 
                  << " Got: " << packet_ptr->compute_checksum() << std::endl;
        break;
      }

      // Check session IDs
      // No need to check on SYN packets
      if (packet_ptr->type == "01" ||
          packet_ptr->type == "02" ||
          packet_ptr->type == "07" ||
          packet_ptr->type == "08" ||
          packet_ptr->type == "09") {
          
        if (protocol_.session_id != packet_ptr->receiver_id ||
            protocol_.receiver_id != packet_ptr->sender_id) {
          std::cout << "[-] Error ~> Server : Mismatched session IDs:" << std::endl
                    << "[.] . Server.session_id = \"" << protocol_.session_id << "\"" << std::endl
                    << "[.] .. packet_ptr->receiver_id = \"" << packet_ptr->receiver_id << "\"" << std::endl
                    << "[.] . Server.receiver_id = \"" << protocol_.receiver_id << "\"" << std::endl
                    << "[.] .. packet_ptr->sender_id = \"" << packet_ptr->sender_id << "\"" << std::endl;
          break;
        }

      }

      // If Handshake packets (SYN-ACK, ACK), check sequence number continuity
      if (packet_ptr->type == "01") {
        int seq_num_x_plus_one = hex_to_dec(packet_ptr->data.substr(26,4));
        if (seq_num_x_plus_one != protocol_.sequence_number + 1) {
          std::cout << "[-] Error ~> Server : (Sequence number x)+1 doesn't match incremented: " << std::endl
                    << "[.] . Expected: " << protocol_.sequence_number + 1 << std::endl
                    << "[.] . Got: " << seq_num_x_plus_one << std::endl;
          break;
                 
        }
      } else if (packet_ptr->type == "02") {
        int seq_num_y_plus_one = hex_to_dec(packet_ptr->data.substr(26,4));
        if (seq_num_y_plus_one != protocol_.sequence_number_y + 1) {
          std::cout << "[-] Error ~> Server : (Sequence number y)+1 doesn't match incremented: " << std::endl
                    << "[.] . Expected: " << protocol_.sequence_number_y + 1 << std::endl
                    << "[.] . Got: " << seq_num_y_plus_one << std::endl;
          break;
      }


      // Process, create, and send next packet in line
      if (packet_ptr->type == "00") {
        SynAckPacket response;
        std::string payload = std::string("") + packet_ptr->sender_id + packet_ptr->data;
        response.craft(protocol_, payload);
        raw_packet = response.encode();
        result = send(sock_, raw_packet.c_str(), strlen(raw_packet.c_str()), 0);
         
      } else if (packet_ptr->type == "01") {
        protocol_.sequence_number_y = hex_to_dec(packet_ptr->data.substr(30,4));
        AckPacket response;
        std::string payload = std::string("") + packet_ptr->sender_id; 
        response.craft(protocol_, payload);
        raw_packet = response.encode();
        result = send(sock_, raw_packet.c_str(), strlen(raw_packet.c_str()), 0);

      } else if (packet_ptr->type == "07") {
        BufferResponsePacket response;
        std::string payload = "" + std::to_string(server_min_buf_size);
        response.craft(protocol_, payload);
        raw_packet = response.encode();
        result = send(sock_, raw_packet.c_str(), strlen(raw_packet.c_str()), 0);
      } 

      if (packet_ptr->type == "02") {
        std::cout << "[+] Connection successfully established." << std::endl;
        handshake_complete = true;
        // ACK, receiver begins buffer exchange
        // Instantly begin request for buffer
        BufferRequestPacket request;
        std::string payload = "";
        request.craft(protocol_, payload);
        raw_packet = request.encode();
        result = send(sock_, raw_packet.c_str(), strlen(raw_packet.c_str()), 0);
        std::cout << "[+] Sent packet [" << result << "]: " << raw_packet << std::endl;
      } else { 
        std::cout << "[+] Sent packet [" << result << "]: " << raw_packet << std::endl; 
      }

    }

  }
 
}
//  BufferRequestPacket buffer_request_server;
//  std::string payload = "";
//  buffer_request_server.craft(protocol_, payload);
//  std::string raw_packet = buffer_request_server.encode();
//  
//  // Send min buffer request
//  result = send(sock_, raw_packet.c_str(), strlen(raw_packet.c_str()), 0);
//  std::cout << "[+] Sent packet [" << result << "]: " << raw_packet << std::endl;
//
//  // Read response
//  result = read(sock_, server_buf, server_min_buf_size); 
//  BufferResponsePacket buffer_response_server;
//  raw_packet = server_buf;
//  buffer_response_server.parse(raw_packet);
//  std::cout << "[+] Received packet [" << result << "]: " << raw_packet << std::endl;
// 
//  // Set new client min buffer size
//  client_min_buf_size = hex_to_dec(buffer_response_server.data); 
//
//  // Wait for buffer request from client
//  result = read(sock_, server_buf, server_min_buf_size);
//  BufferRequestPacket buffer_request_client;
//  raw_packet = server_buf;
//  buffer_request_client.parse(raw_packet);
//  std::cout << "[+] Received packet [" << result << "]:" << raw_packet << std::endl; 
//  
//  if (buffer_request_client.type == "07") {
//    // Buffer request received
//    BufferResponsePacket buffresponse = protocol_.craft_min_buffer_response_packet(server_min_buf_size);
//    raw_packet = response.encode();
//    result = send(sock_, raw_packet.c_str(), strlen(raw_packet.c_str()), 0);
//    std::cout << "[+] Sent packet [" << result << "]: " << raw_packet << std::endl;
//  }
//
//  // Handshake
//  // Send SYN packet
//  Packet syn_packet = protocol_.craft_syn_packet();
//  int seq_num = protocol_.hex_to_dec(syn_packet.data);
//
//  std::string raw_syn_packet = syn_packet.encode();
//
//  result = send(sock_, raw_syn_packet.c_str(), client_min_buf_size, 0);
//  std::cout << "[+] Sent packet [" << result << "]: " << raw_syn_packet << std::endl; 
//
//  result = read(sock_, server_buf, server_min_buf_size);
//  raw_packet = server_buf;
//  Packet syn_ack_packet;
//  syn_ack_packet.parse(raw_packet);
//
//  std::cout << "[+] Received packet [" << result << "]: " << raw_packet << std::endl;
//
//  if (syn_ack_packet.type == "01") {
//    // SYN-ACK received
//
//    // Check checksums
//    if (syn_ack_packet.checksum != syn_ack_packet.compute_checksum()) {
//      protocol_.error(std::string("ERROR [Server <- SYN-ACK] - Checksum not matching:") +
//                      "...Expected: " + syn_ack_packet.checksum + "\n...Received: " +
//                      syn_ack_packet.compute_checksum());
//      return;
//    }
//
//
//    // Check session IDs
//    if (syn_ack_packet.receiver_id != protocol_.session_id_) {
//      // terminate, incorrect receiver_id
//      protocol_.error("ERROR [Server <- SYN-ACK] - Incorrect receiver id, expected: " + 
//                      protocol_.session_id_ + " received: " + syn_ack_packet.receiver_id);
//      return;
//    }
//
//    if (syn_ack_packet.sender_id != syn_packet.receiver_id) {
//      // terminate, incorrect session_id
//      protocol_.error("ERROR [Server <- SYN_ACK] - Incorrect sender id, expected: " + 
//                      syn_packet.receiver_id + " received: " + syn_ack_packet.sender_id);
//      return;
//    }
//
//    // Check sequence number x increment 
//    if (protocol_.hex_to_dec(syn_packet.data) + 1 != 
//        (protocol_.hex_to_dec(syn_ack_packet.data.substr(0,4)))) {
//      // terminate, invalid sequence number increment
//      protocol_.error(std::string("ERROR [Server <- SYN_ACK] -") + 
//                      " Incorrect sequence number increment, expected: " + 
//                      std::to_string(protocol_.hex_to_dec(syn_ack_packet.data.substr(0,4)))+ 
//                      " received: " +
//                      std::to_string(protocol_.hex_to_dec(syn_packet.data) + 1));
//      return;
//    }
//
//    int seq_num_y = 0;
//    seq_num_y = protocol_.hex_to_dec(syn_ack_packet.data.substr(4,4));
//    Packet ack_packet(protocol_.craft_ack_packet(seq_num_y, syn_ack_packet));
//
//    std::string raw_ack_packet = ack_packet.encode();
//
//    // Send ACK packet
//    result = send(sock_, raw_ack_packet.c_str(), client_min_buf_size, 0);
//    std::cout << "[+] Sent packet [" << result << "]: " << raw_ack_packet << std::endl;
//    std::cout << "[+] Connection successfully established (Handshake complete)." << std::endl;
//  }
//
//  // Use case: Simulated buffer overflow
//  std::string message = std::string("This is a very long message, longer than the size") +
//                                    "of the buffer that is supposed to receive it. This " + 
//                                    "is created to test the exachange of buffer sizes " + 
//                                    "between a server and client.";  
//
//  Packet buffer_test_packet = protocol_.craft_data_packet(message, syn_ack_packet.sender_id);
//  std::cout << "[*] Packing message in packet: \"" << message << "\"" << std::endl;
//  raw_packet = buffer_test_packet.encode();
//  result = send(sock_, raw_packet.c_str(), (size_t)client_min_buf_size, 0);  
//  std::cout << "[+] Sent packet [" << result << "]: " << raw_packet << std::endl;
//


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
