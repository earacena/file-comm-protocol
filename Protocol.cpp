// Name: Emanuel Aracena Beriguete
// Date: February 14, 2020
// Filename: Protocol.cpp
// Description: Protocol for single client/single server transmission.

#include "Protocol.hpp"

Protocol::Protocol() { }

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
  logger_.save_log();
}

/////// Handshake helpers
// SYN
// @return: string in syn packet format
std::string Protocol::craft_syn_packet() {
  // syn packet format:
  // sender session id (2 bytes) : receiver session id (2 bytes) : SYN (1 byte - 00) : Sequence number
  //   (1 bytes, random number from 0-255)
  // ex: 2A : D1 : 00 : A1 B1
  // NOTE: initiator decides the receiver session id
  std::string packet;
  
  // Since this is the first communication, create session ids
  session_id_ = random_hex_str(2);
  std::string receiver_session_id(random_hex_str(2));

  packet.append(session_id_);
  packet.append(receiver_session_id);
  packet.append("00");
  sequence_number_ = random_number(4);
  std::string hex_seq_num(dec_to_hex(sequence_number_));
  if (hex_seq_num.length() < 4)
    hex_seq_num = "0" + hex_seq_num;

  packet.append(hex_seq_num);
  logger_.record_event(std::string("SYN packet crafted::") +
                       "\n\t\t\t- Raw packet: " + packet +
                       "\n\t\t\t- Session id (Sender): " + session_id_ +
                       "\n\t\t\t- Receiver id: " + receiver_session_id +
                       "\n\t\t\t- Packet type: (SYN) 00" +
                       "\n\t\t\t- Sequence number (x): " + hex_seq_num);
  return packet; 
}


// SYN-ACK
std::string Protocol::craft_syn_ack_packet(int sequence_number_x, const std::string & syn_packet) {
  // SYN-ACK packet format
  // sender session id (2 bytes) : receiver session id (2 bytes) : SYN-ACK (1 byte - 01) :
  //  Sequence number x + 1 : Sequence number y
  //   (1 bytes, random number from 0-255)
  // ex: D1 : 2A  : 01 : A1 B2 : FF 00 
  // NOTE: initiator decides the receiver session id

  // Set session id given by initiator, eventually make Session class handle this
  std::string packet;

  // Extract given receiver id
  std::string session_id("");
  session_id.append(syn_packet.substr(2,2));
  session_id_ = session_id;
  packet.append(session_id_);

  // Extract sender id
  std::string sender_id("");
  sender_id.append(syn_packet.substr(0,2));
  packet.append(sender_id);

  // Packet type
  packet.append("01");

  // Increment sequence number x
  sequence_number_x += 1;
  std::string hex_seq_num_x(dec_to_hex(sequence_number_x));
  if (hex_seq_num_x.length() < 4)
    hex_seq_num_x = "0" + hex_seq_num_x;
  packet.append(hex_seq_num_x);

  // Generate sequence number y
  sequence_number_ = random_number(4);
  std::string hex_seq_num_y(dec_to_hex(sequence_number_));
  if (hex_seq_num_y.length() < 4)
    hex_seq_num_y = "0" + hex_seq_num_y;
  packet.append(hex_seq_num_y);


  logger_.record_event(std::string("SYN-ACK packet crafted::") +
                       "\n\t\t\t- Raw packet: " + packet +
                       "\n\t\t\t- Session id (Sender): " + session_id_ +
                       "\n\t\t\t- Receiver id: " + sender_id +
                       "\n\t\t\t- Packet type: (SYN-ACK) 01" +
                       "\n\t\t\t- Sequence number (x+1): " + hex_seq_num_x  +
                       "\n\t\t\t- Sequence number y: " + hex_seq_num_y);
  return packet; 

}


// ACK
std::string Protocol::craft_ack_packet(int sequence_number_y, const std::string & syn_ack_packet) {
  // ACK packet format
  // sender session id (1 bytes) : receiver session id (1 bytes) : ACK (1 byte - 02) :
  //  Sequence number y+1
  //   (1 bytes, random number from 0-255)
  // ex: D1 : 2A : 02 : FF 01
  // NOTE: initiator decides the receiver session id

  // Set session id given by initiator, eventually make Session class handle this
  std::string packet;

  // Extract given receiver id
  std::string sender_id("");
  sender_id.append(syn_ack_packet.substr(2,2));
  packet.append(sender_id);

  // Extract receiver id
  std::string receiver_id("");
  receiver_id.append(syn_ack_packet.substr(0,2));
  packet.append(receiver_id);

  // Packet type
  packet.append("02");

  // Generate sequence number y
  std::string hex_seq_num_y(dec_to_hex(sequence_number_y + 1));
  if (hex_seq_num_y.length() < 4)
    hex_seq_num_y = "0" + hex_seq_num_y;
  packet.append(hex_seq_num_y);


  logger_.record_event(std::string("ACK packet crafted::") +
                       "\n\t\t\t- Raw packet: " + packet +
                       "\n\t\t\t- Session id (Sender): " + sender_id +
                       "\n\t\t\t- Receiver id: " + receiver_id +
                       "\n\t\t\t- Packet type: (ACK) 02" +
                       "\n\t\t\t- Sequence number y: " + hex_seq_num_y);
  return packet; 

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
    std::cout << "rand_val: " << rand_val << " result: " << result << std::endl;
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
