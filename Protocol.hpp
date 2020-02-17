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

#include "Logger.hpp"

class Protocol {
public:
  Protocol(const bool logging, const int mode);
  ~Protocol();

  // Handshake
  // return: 1 if successful, -1 otherwise
  int attempt_handshake(const std::string & syn_packet);

  // Identify
	int attempt_identify();

  // Pre-transmission
	int attempt_pre_transmit();

  // Transmission

  // Error-checking


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
  std::string craft_syn_packet();
  void send_syn_packet(const std::string & packet);
	std::string receive_syn_packet();
  // SYN-ACK
  std::string craft_syn_ack_packet(int sequence_number_x, const std::string & syn_packet);
  void send_syn_ack_packet(const std::string & packet);
	std::string receive_syn_ack_packet();
  // ACK
  std::string craft_ack_packet(int sequence_number_y);
  void send_ack_packet(const std::string & packet);
	std::string receive_ack_packet();
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
