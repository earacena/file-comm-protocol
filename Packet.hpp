// Name: Emanuel Aracena Beriguete
// Date: February 18, 2020
// Filename: Packet.hpp
// Description: A container for various packet types/formats.

#ifndef PACKET_HPP
#define PACKET_HPP

#include <string>
#include <algorithm>

#include "Protocol.hpp"


// Packet format/fields/sizes
// Example:
// * : * : * : 0001 : 0002 : 2A : B1 : 01 : * : data
// Packet size (for now)       2 bytes : 4
// Start-by (for data section) 1 byte  : 2
// End-by (for data section)   1 byte  : 2
// Packet number               2 bytes : 4
// Total packets               2 bytes : 4
// Sender id                   1 byte  : 2
// Receiver id                 1 byte  : 2
// Type                        1 byte  : 2
// Checksum                    2 bytes : 4
// Data                        * byte(s) :*
// Packet size = 26 + (data length)

// Packet Mapping
// 00 - SYN     |
// 01 - SYN-ACK | Handshake
// 02 - ACK     |
// 03 - PKU - RESERVED FOR IDENTITY |
// 04 - PKS - RESERVED FOR IDENTITY | Identity
// 05 - EKU - RESERVED FOR IDENTITY |
// 06 - EKS - RESERVED FOR IDENTITY |
// 07 - BRQ - Buffer size request  |
// 08 - BRS - Buffer size response | Parameter/Pre-Transmit
// 09 - DAT - General Use/Data packet | ETC 


class Packet {
public:

  Packet();

  // Craft
  virtual void craft(const std::string & payload) = 0;
  
  // Read raw packet data into Packet format
  virtual void parse(const std::string & raw_packet);
    
  // Return Packet data in raw (hex) data for submission
  std::string encode();
  
  // Calculate checksum, done in last step, returns hex value
  std::string compute_checksum();

  int packet_size;            
  int start_by;               
  int end_by;                 
  int packet_num;             
  int total_packets;          
  std::string sender_id;      
  std::string receiver_id;    
  std::string type;           
  std::string checksum;       
  std::string data;           

};

class SynPacket : public Packet {
  virtual void craft(const Protocol & proto, const std::string & payload) override;

};

class SynAckPacket : public Packet {
  virtual void craft(const Protocol & proto, const std::string & payload) override;

};

class AckPacket : public Packet {
  virtual void craft(const Protocol & proto, const std::string & payload) override;

};

class BufferRequestPacket : public Packet {
  virtual void craft(const Protocol & proto, const std::string & payload) override;

};

class BufferResponsePacket : public Packet {
  virtual void craft(const Protocol & proto, const std::string & payload) override;

};

class DataPacket : public Packet {
  virtual void craft(const Protocol & proto, const std::string & payload) override;

}
#endif // PACKET_HPP
