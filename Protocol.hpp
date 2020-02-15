// Name: Emanuel Aracena Beriguete
// Date: February 14, 2020
// Filename: Protocol.hpp
// Description: Protocol for communication between single client/single server
//              transmission.

#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <iostream>
#include <string>

#include "Logger.hpp"

class Protocol {
  Protocol(const bool logging);

  std::string str_to_hex(const std::string & unencoded);
  std::string hex_to_str(const std::string & encoded);

  // Handshake
  // return: 1 if successful, -1 otherwise
  int attempt_handshake();

  // Identify
	int attempt_identify();

  // Pre-transmission
	int attempt_pre_transmit();

  // Transmission

  // Error-checking

private:
  // Allow logger to record protocol events
  friend class Logger;
  bool logging_;

  // Used to differentiate multiple servers/clients on one physical computer
  // Not used for identity stage/layer
  std::string session_id_;

	// sequence numbers must be randomized to avoid collision with other connections
	int sequence_number_;

	/////// Hex conversion helpers
  int hex_value(const char hex_digit);
  std::string str_to_hex(const std::string & unencoded);
  std::string hex_to_str(const std::string & encoded);
	/////////

	/////// Handshake helpers
  // SYN
  std::string craft_syn_packet();
  void send_syn_packet(const std::string & packet);
	std::string receive_syn_packet();
  // SYN-ACK
  std::string craft_syn_ack_packet(int sequence_number_x);
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
	std::vector<int> data(10);
};

#endif // PROTOCOL_HPP
