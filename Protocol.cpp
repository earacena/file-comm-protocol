// Name: Emanuel Aracena Beriguete
// Date: February 14, 2020
// Filename: Protocol.cpp
// Description: Protocol for single client/single server transmission.

#include "Protocol.hpp"

Protocol::Protocol(const bool logging, const int mode) { 
  logging_ = logging;
  if (logging_) {
    std::time_t time = std::time(nullptr);
    std::string filename;
    filename = "./logs/";
    filename.append(std::to_string(static_cast<long int>(time)));
    // if mode is 1, server otherwise client
    filename = filename + (mode ? "-server.txt" : "-client.txt");
    logger_.set_filename(filename);
  }
}

Protocol::~Protocol() {
  logger_.save_log();
}

/////// Handshake helpers
// SYN
// @return: string in syn packet format
std::string Protocol::craft_syn_packet() {
  // syn packet format:
  // sender session id (2 bytes) : receiver session id (2 bytes) : SYN (1 byte - 00) : Sequence number
  //   (1 bytes, random number from 0-255)
  // ex: 2A 45 : D1 43 : 00 : A1
  // NOTE: initiator decides the receiver session id
  std::string packet;
  
  // Since this is the first communication, create session ids
  session_id_ = random_hex_str(4);
  std::string receiver_session_id(random_hex_str(4));

  packet.append(session_id_);
  packet.append(receiver_session_id);
  packet.append("00");
  sequence_number_ = random_number(4);
  std::string hex_seq_num(dec_to_hex(sequence_number_));
  packet.append(hex_seq_num);
  logger_.record_event(std::string("Syn packet crafted::") +
                       "\n\t\t\t- Raw packet: " + packet +
                       "\n\t\t\t- Session id (Sender): " + session_id_ +
                       "\n\t\t\t- Receiver id: " + receiver_session_id +
                       "\n\t\t\t- Packet type: (SYN) 00"
                       "\n\t\t\t- Sequence number (x): " + hex_seq_num);
  return packet; 
}

void Protocol::send_syn_packet(const std::string & packet) {

}

std::string Protocol::receive_syn_packet() {
  std::string test;
  return test;
}
// SYN-ACK
std::string Protocol::craft_syn_ack_packet(int sequence_number_x) {
  std::string test;
  return test;

}

void Protocol::send_syn_ack_packet(const std::string & packet) {

}

std::string Protocol::receive_syn_ack_packet() {
  std::string test;
  return test;

}
// ACK
std::string Protocol::craft_ack_packet(int sequence_number_y) {
  std::string test;
  return test;

}

void Protocol::send_ack_packet(const std::string & packet) {

}

std::string Protocol::receive_ack_packet() {
  std::string test;
  return test;

}
////////




////// Hex conversion helpers ///////
std::string Protocol::str_to_hex(const std::string & unencoded) {
  static const std::string hex_digits("0123456789ABCDEF");

  std::string encoded;
  // every character is a hex pair, reserve double the size of message
  encoded.reserve(unencoded.size() * 2);

  for (unsigned char ch : unencoded) {
    // Find hex for most significant 4 bits 
    encoded.push_back(hex_digits.at(ch >> 4));

    // c & 15 is same as 00001111, finding hex for least signif. 4 bits
    encoded.push_back(hex_digits.at(ch & 15));
  }

  return encoded;
}

std::string Protocol::hex_to_str(const std::string & encoded) {
  size_t len = encoded.length();

  // If odd length and invalid input since it must be in hex pairs
  if (len % 2 != 0) 
    return "ERROR, invalid length";

  std::string unencoded;
  unencoded.reserve(len/2);
  for(auto it = unencoded.begin(); it != unencoded.end(); ) {
    int high_bits = hex_value(*it++);
    int low_bits = hex_value(*it++);
    // Combine into pair
    unencoded.push_back(high_bits << 4 | low_bits);
  }

  return unencoded;
}

std::string Protocol::random_hex_str(int length) {
  std::string result("");
  std::string hex_digits("0123456789ABCDEF");

  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<> dist(0, 15);
  
  int rand_val = 0;
  for (int i = 0; i < length; ++i)
    rand_val = static_cast<int>(dist(mt));
    result.append(std::to_string(hex_digits.at(rand_val)));

  return result;
}

int Protocol::random_number(int length) {
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<> dist(0, 10000);
  return dist(mt);
}

int Protocol::hex_value(const char hex_digit) {
  if (hex_digit >= '0' && hex_digit <= '9')
    return hex_digit - '0';

  if (hex_digit >= 'A' && hex_digit <= 'F')
    return (hex_digit - 'A') + 10;

  if (hex_digit >= 'a' && hex_digit <= 'f')
    return (hex_digit - 'a') + 10;

  // If value not returned then invalid character given
  return -1;
}

std::string Protocol::dec_to_hex(int number) {
  std::stringstream stream;
  stream << std::hex << number;
  return stream.str();
}

////////////////////


//////// Logging specific
void Protocol::error(const std::string & error_msg) {
  logger_.record_event(error_msg);
}
