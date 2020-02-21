// Name: Emanuel Aracena Beriguete
// Date: February 14, 2020
// Filename: Protocol.cpp
// Description: Protocol for single client/single server transmission.

#include "Protocol.hpp"

Protocol::Protocol() {
  logging_ = false;
}

Protocol::Protocol(const bool logging, const int mode) { 
  logging_ = logging;
  if (logging_) {
    std::time_t time = std::time(nullptr);
    std::string filename;
    filename = "./logs/";
    // if mode is 1, server otherwise client
    filename = filename + (mode ? "server-" : "client-");
    filename.append(std::to_string(static_cast<long int>(time)));
    filename.append(".txt");
    logger_.set_filename(filename);
  }
}

Protocol::~Protocol() {
  if (logging_)
    logger_.save_log();
}

/////// Handshake helpers
// SYN
// @return: string in syn packet format
Packet Protocol::craft_syn_packet() {
  // syn packet format:
  // sender session id (2 bytes) : receiver session id (2 bytes) : SYN (1 byte - 00) : Sequence number
  //   (1 bytes, random number from 0-255)
  // ex: 2A : D1 : 00 : A1 B1
  // NOTE: initiator decides the receiver session id
  Packet syn_packet;
  
  // Since this is the first communication, create session ids
  session_id_ = random_hex_str(2);
  syn_packet.sender_id = session_id_;

  std::string receiver_session_id(random_hex_str(2));
  syn_packet.receiver_id = receiver_session_id;
  syn_packet.type = "00";

  sequence_number_ = random_number(4);
  std::string hex_seq_num(dec_to_hex(sequence_number_));
  if (hex_seq_num.length() < 4)
    hex_seq_num = "0" + hex_seq_num;

  syn_packet.data = hex_seq_num;
  syn_packet.start_by = 26;
  syn_packet.end_by = syn_packet.start_by + syn_packet.data.length();
  syn_packet.packet_size = 26 + syn_packet.data.length();
  syn_packet.packet_num = 1;
  syn_packet.total_packets = 1;
  syn_packet.checksum = syn_packet.compute_checksum();

  logger_.record_event(std::string("\nSYN packet crafted::") +
                       "\n\t- Raw packet:\t\t" + syn_packet.encode() +
                       "\n\t- Packet size:\t\t" + dec_to_hex(syn_packet.packet_size) +
                       "\n\t- Start-by:\t\t" + dec_to_hex(syn_packet.start_by) +
                       "\n\t- End-by:\t\t" + dec_to_hex(syn_packet.end_by) +
                       "\n\t- Packet # (out of n):\t" + dec_to_hex(syn_packet.packet_num) +
                       "\n\t- Total packets (n):\t" + dec_to_hex(syn_packet.total_packets) +
                       "\n\t- Session id (Sender):\t" + syn_packet.sender_id +
                       "\n\t- Receiver id:\t\t" + syn_packet.receiver_id +
                       "\n\t- Packet type:\t\t" + syn_packet.type +
                       "\n\t- Checksum:\t\t" + syn_packet.checksum +
                       "\n\t- Data:" +
                       "\n\t... Seq. num. (x):\t" + syn_packet.data);
  return syn_packet; 
}


// SYN-ACK
Packet Protocol::craft_syn_ack_packet(int sequence_number_x, const Packet & syn_packet) {
  // SYN-ACK packet format
  // NOTE: initiator decides the receiver session id

  // Set session id given by initiator, eventually make Session class handle this
  Packet syn_ack_packet;

  // Extract given ids
  session_id_ = syn_packet.receiver_id;
  syn_ack_packet.sender_id = syn_packet.receiver_id;
  syn_ack_packet.receiver_id = syn_packet.sender_id;


  // Packet type
  syn_ack_packet.type = "01";

  // Increment sequence number x
  sequence_number_x += 1;
  std::string hex_seq_num_x(dec_to_hex(sequence_number_x));
  if (hex_seq_num_x.length() < 4)
    hex_seq_num_x = "0" + hex_seq_num_x;
  syn_ack_packet.data.append(hex_seq_num_x);

  // Generate sequence number y
  sequence_number_ = random_number(4);
  std::string hex_seq_num_y(dec_to_hex(sequence_number_));
  if (hex_seq_num_y.length() < 4)
    hex_seq_num_y = "0" + hex_seq_num_y;
  syn_ack_packet.data.append(hex_seq_num_y);

  syn_ack_packet.packet_size = 26 + syn_ack_packet.data.length();
  syn_ack_packet.start_by = 26;
  syn_ack_packet.end_by = syn_ack_packet.data.length() + syn_ack_packet.start_by;
  syn_ack_packet.packet_num = 1;
  syn_ack_packet.total_packets = 1;
  syn_ack_packet.checksum = syn_ack_packet.compute_checksum();

  logger_.record_event(std::string("\nSYN-ACK packet crafted::") +
                       "\n\t- Raw packet:\t\t" + syn_ack_packet.encode() +
                       "\n\t- Packet size:\t\t" + dec_to_hex(syn_ack_packet.packet_size) +
                       "\n\t- Start-by:\t\t" + dec_to_hex(syn_ack_packet.start_by) +
                       "\n\t- End-by:\t\t" + dec_to_hex(syn_ack_packet.end_by) +
                       "\n\t- Packet # (out of n):\t" + dec_to_hex(syn_ack_packet.packet_num) +
                       "\n\t- Total packets (n):\t" + dec_to_hex(syn_ack_packet.total_packets) +
                       "\n\t- Session id (Sender):\t" + syn_ack_packet.sender_id +
                       "\n\t- Receiver id:\t\t" + syn_ack_packet.receiver_id +
                       "\n\t- Packet type:\t\t" + syn_ack_packet.type +
                       "\n\t- Checksum:\t\t" + syn_ack_packet.checksum +
                       "\n\t- Data:" +
                       "\n\t... Seq. num. (x+1):\t" + syn_ack_packet.data.substr(0,4) +
                       "\n\t... Seq. num. (y):\t" + syn_ack_packet.data.substr(4));

  return syn_ack_packet; 

}

// ACK
Packet Protocol::craft_ack_packet(int sequence_number_y, const Packet & syn_ack_packet) {
  // ACK packet format
  // sender session id (1 bytes) : receiver session id (1 bytes) : ACK (1 byte - 02) :
  //  Sequence number y+1
  //   (1 bytes, random number from 0-255)
  // ex: D1 : 2A : 02 : FF 01
  // NOTE: initiator decides the receiver session id

  // Set session id given by initiator, eventually make Session class handle this
  Packet ack_packet;

  // Extract given ids
  ack_packet.sender_id = syn_ack_packet.receiver_id;
  ack_packet.receiver_id = syn_ack_packet.sender_id;

  // Packet type
  ack_packet.type = "02";

  // Generate sequence number y
  std::string hex_seq_num_y(dec_to_hex(sequence_number_y + 1));
  if (hex_seq_num_y.length() < 4)
    hex_seq_num_y = "0" + hex_seq_num_y;
  ack_packet.data.append(hex_seq_num_y);

  ack_packet.packet_size = 26 + ack_packet.data.length();
  ack_packet.start_by = 26;
  ack_packet.end_by = ack_packet.start_by + ack_packet.data.length();
  ack_packet.packet_num = 1;
  ack_packet.total_packets = 1;
  ack_packet.checksum = ack_packet.compute_checksum();

  logger_.record_event(std::string("\nACK packet crafted::") +
                       "\n\t- Raw packet:\t\t" + ack_packet.encode() +
                       "\n\t- Packet size:\t\t" + dec_to_hex(ack_packet.packet_size) +
                       "\n\t- Start-by:\t\t" + dec_to_hex(ack_packet.start_by) +
                       "\n\t- End-by:\t\t" + dec_to_hex(ack_packet.end_by) +
                       "\n\t- Packet # (out of n):\t" + dec_to_hex(ack_packet.packet_num) +
                       "\n\t- Total packets (n):\t" + dec_to_hex(ack_packet.total_packets) +
                       "\n\t- Session id (Sender):\t" + ack_packet.sender_id +
                       "\n\t- Receiver id:\t\t" + ack_packet.receiver_id +
                       "\n\t- Packet type:\t\t" + ack_packet.type +
                       "\n\t- Checksum:\t\t" + ack_packet.checksum +
                       "\n\t- Data:" +
                       "\n\t... Seq. num. (y+1):\t" + ack_packet.data);

  return ack_packet; 

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
  for (int i = 0; i < length; ++i) {
    rand_val = dist(mt);
    result.append(hex_digits.substr(rand_val,1));
  }
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

int Protocol::hex_to_dec(const std::string & encoded) {
  return std::stoi(encoded, nullptr, 16);
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
