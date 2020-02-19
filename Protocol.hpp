// Name: Emanuel Aracena Beriguete
// Date: February 14, 2020
// Filename: Protocol.hpp
// Description: Protocol for communication between single client/single server
//              transmission.

#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <iostream>
#include <string>
#include <random>
#include <iomanip>
#include <sstream>

#include "Packet.hpp"
#include "Logger.hpp"

class Packet;

class Protocol {
public:
  Protocol();
  Protocol(const bool logging, const int mode);
  ~Protocol();

  // Logging specific
  void error(const std::string & error_msg);
 
//private:

  // Allow logger to record protocol events
  friend class Logger;
  bool logging_;
  Logger logger_;

  // Used to differentiate multiple servers/clients on one physical computer
  // Not used for identity stage/layer
  std::string session_id_;

	// sequence numbers must be randomized to avoid collision with other connections
	int sequence_number_;

	/////// Hex conversion helpers
  int hex_value(const char hex_digit);
  std::string str_to_hex(const std::string & unencoded);
  std::string hex_to_str(const std::string & encoded);
  std::string random_hex_str(int length);
  int random_number(int length);
  int hex_to_dec(const std::string & encoded);
  std::string dec_to_hex(int number);  
	/////////

	/////// Handshake helpers
  // SYN
  Packet craft_syn_packet();
  // SYN-ACK
  Packet craft_syn_ack_packet(int sequence_number_x, const Packet & syn_packet);
  // ACK
  Packet craft_ack_packet(int sequence_number_y, const Packet & syn_ack_packet);
	////////

	/////// Identify helpers
  void generate_keys();
  std::string encapsulate_public_key(const std::string & public_key, const std::string & packet);
  void send_public_key_packet();
	////////

	// Pre-Transmit methods/members
	// data vector with parameters for recipient
	// data[0] - max buffer/message size, request: MBUF
	// data[1] - max memory, request: MMEM
	// ...
	std::string craft_data_packet(const std::string & request);
};

#endif // PROTOCOL_HPP
