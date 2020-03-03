// Name: Emanuel Aracena Beriguete
// Date: February 14, 2020
// Filename: Protocol.cpp
// Description: Protocol for single client/single server transmission.

#include "Protocol.hpp"

Protocol::Protocol() {
//  logging_ = false;
}

Protocol::Protocol(const bool logging, const int mode) { 
  //logging_ = logging;
  //if (logging_) {
  //  std::time_t time = std::time(nullptr);
  //  std::string filename;
  //  filename = "./logs/";
  //  // if mode is 1, server otherwise client
  //  filename = filename + (mode ? "server-" : "client-");
  //  filename.append(std::to_string(static_cast<long int>(time)));
  //  filename.append(".txt");
  //  logger.set_filename(filename);
  //}
}

Protocol::~Protocol() {
  //if (logging_)
  //  logger.save_log();
}

void Protocol::loop(const int socket) {
  std::cout << ".\n"; 
  

  std::string raw_packet = "";
  int result = 0; 
  //-------------------------------------------------------------
  // SEND FIRST, Node initiates connection
  //-------------------------------------------------------------
  // Normally a SYN packet would be only SEND FIRST action on first loop
  // but since server is passively listening, client will be sending SYN
  // most of the time, unless current protocol changes

  if (!connection_initiated) {
    SynPacket syn_packet;
    std::string payload = "";
    syn_packet.craft(*this, payload);
    session_id = syn_packet.sender_id;
    receiver_id = syn_packet.receiver_id;
    raw_packet = syn_packet.encode();
    result = send(socket, raw_packet.c_str(), receiver_min_buf_size, 0);
    std::cout << "[+] Sent packet [" << result << "]: " << raw_packet << std::endl;
    connection_initiated = true;
  }
  //-------------------------------------------------------------
  // RECEIVE FIRST, Node receives requests/packets first 
  //-------------------------------------------------------------

  // Polling
  std::cout << "[*] Polling (1 second timeout)..." << std::endl;

  // poll ufds, 1 fd, 1000 ms
  int retval = poll(ufds, 1, 1000);
  
  if (retval < 0) {
    std::cout << "[-] Error ~> Poll()" << std::endl;
    perror("poll");
    exit(EXIT_FAILURE);
  } else if (retval == 0) {
    std::cout << "[-] Timeout occured (1s)! Terminating due to unresponsive connection..."
              << std::endl;
    exit(EXIT_FAILURE);
  } // else, continue

  // retrieve packet

  if (ufds[0].revents & POLLIN) {
    result = recv(socket, buffer, min_buf_size, 0);
  } else {
    std::cout << "[-] Error ~> checking events, falling back to recv." << std::endl;
    result = recv(socket, buffer, min_buf_size, 0);
  }

  if (result < 0) {
    // Error received
    std::cout << "[-] Error ~> recv()" << std::endl;
    return;
  } else if (result == 0) {
    // remote client closed connection
    std::cout << "[-] Error ~> Remote client closed connection." << std::endl;
    exit(EXIT_SUCCESS);
  } else {
    // result is set to number of transmitted bytes
    raw_packet = buffer;
    std::cout << "[+] Received packet [" << result << "]: " << raw_packet << std::endl;
    
    // Extract raw packet type to determine how it's parsed
    if (raw_packet == "") {
      std::cout << "[-] Empty packet received, terminating..." << std::endl;
      exit(EXIT_SUCCESS);
    }
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
      std::cout << "[-] Error ~> Invalid packet type [" << packet_type << "]" << std::endl;
      return;
    }
    // Parse into appropirate object
    packet_ptr->parse(raw_packet);

    // Verify checksum
    if (packet_ptr->checksum != packet_ptr->compute_checksum()) {
      std::cout << "[-] Error ~> Invalid Checksum, Exp: " << packet_ptr->checksum 
                << " Got: " << packet_ptr->compute_checksum() << std::endl;
      return;
    }
  

    // Check session IDs
    // If SYN, set session IDs
    if (packet_ptr->type == "00") {
      session_id = packet_ptr->receiver_id; 
      receiver_id = packet_ptr->sender_id;
    }

    // No need to check on SYN packets
    if (packet_ptr->type == "01" ||
        packet_ptr->type == "02" ||
        packet_ptr->type == "07" ||
        packet_ptr->type == "08" ||
        packet_ptr->type == "09") {
        
      if (session_id != packet_ptr->receiver_id ||
          receiver_id != packet_ptr->sender_id) {
        std::cout << "[-] Error ~> Mismatched session IDs:" << std::endl
                  << "[.] . Server.session_id = \"" << session_id << "\"" << std::endl
                  << "[.] .. packet_ptr->receiver_id = \"" << packet_ptr->receiver_id << "\"" << std::endl
                  << "[.] . Server.receiver_id = \"" << receiver_id << "\"" << std::endl
                  << "[.] .. packet_ptr->sender_id = \"" << packet_ptr->sender_id << "\"" << std::endl;
        return;
      }

    }
    
    // If Handshake packets (SYN-ACK, ACK), check sequence number continuity
    if (packet_ptr->type == "01") {
      int seq_num_x_plus_one = hex_to_dec(packet_ptr->data.substr(0,4));
      if (seq_num_x_plus_one != sequence_number + 1) {
        std::cout << "[-] Error ~> (Sequence number x)+1 doesn't match incremented: " << std::endl
                  << "[.] . Expected: " << sequence_number + 1 << std::endl
                  << "[.] . Got: " << seq_num_x_plus_one << std::endl;
        return;
               
      }
    } else if (packet_ptr->type == "02") {
      int seq_num_y_plus_one = hex_to_dec(packet_ptr->data.substr(0,4));
      if (seq_num_y_plus_one != sequence_number + 1) {
        std::cout << "[-] Error ~> (Sequence number y)+1 doesn't match incremented: " << std::endl
                  << "[.] . Expected: " << sequence_number_y + 1 << std::endl
                  << "[.] . Got: " << seq_num_y_plus_one << std::endl;
        return;
      }
    }

    // Process, create, and send next packet in line
    if (packet_ptr->type == "00") {
      SynAckPacket response;
      std::string payload = std::string("") + packet_ptr->sender_id + packet_ptr->data;
      response.craft(*this, payload);
      raw_packet = response.encode();
      result = send(socket, raw_packet.c_str(), receiver_min_buf_size, 0);
       
    } else if (packet_ptr->type == "02") {
      std::cout << "[+] Connection successfully established." << std::endl;
      // ACK, receiver begins buffer exchange
      // Instantly begin request for buffer
      BufferRequestPacket request;
      std::string payload = "" + packet_ptr->sender_id;
      request.craft(*this, payload);
      raw_packet = request.encode();
      result = send(socket, raw_packet.c_str(), receiver_min_buf_size, 0);
      asked_for_buffer = true;

    } else if (packet_ptr->type == "01") {
      sequence_number_y = hex_to_dec(packet_ptr->data.substr(4,4));
      AckPacket response;
      std::string payload = std::string("") + packet_ptr->sender_id;
      response.craft(*this, payload);
      raw_packet = response.encode();
      result = send(socket, raw_packet.c_str(), receiver_min_buf_size, 0);
      std::cout << "[+] Connection successfully established. ACK sending..." << std::endl;

    } else if (packet_ptr->type == "07") {
      if (!asked_for_buffer) {
        BufferRequestPacket request;
        std::string payload = "" + packet_ptr->sender_id;
        request.craft(*this, payload);
        raw_packet = request.encode();
        result = send(socket, raw_packet.c_str(), receiver_min_buf_size, 0);
        asked_for_buffer = true;
      } else {
        BufferResponsePacket response;
        std::string payload = "" + packet_ptr->sender_id + dec_to_hex(min_buf_size);
        response.craft(*this, payload);
        raw_packet = response.encode();
        result = send(socket, raw_packet.c_str(), receiver_min_buf_size, 0);

      }

    } else if (packet_ptr->type == "08") {
      receiver_min_buf_size = hex_to_dec(packet_ptr->data);

    } else {
      std::cout << "[-] No routine for that packet type." << std::endl;
    } 

      std::cout << "[+] Sent packet [" << result << "]: " << raw_packet << std::endl; 

  }

} 

// ---------------------- Logging specific ---------------------
void Protocol::error(const std::string & error_msg) {
  std::cout << error_msg << std::endl;
  //logger.record_event(error_msg);
}
