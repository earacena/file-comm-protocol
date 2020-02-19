// Name: Emanuel Aracena Beriguete
// Date: February 18, 2020
// Filename: Packet.hpp
// Description: A container for various packet types/formats.

#ifndef PACKET_HPP
#define PACKET_HPP

#include <string>

#include "Protocol.hpp"


// Packet format/fields/sizes
// Example:
// * : * : * : 0001 : 0002 : 2A : B1 : 01 : * : data
// Packet size (for now)       2 bytes:
// Start-by (for data section) 1 byte :
// End-by (for data section)   1 byte :
// Packet number               2 bytes:
// Total packets               2 bytes:
// Sender id                   1 byte :
// Receiver id                 1 byte :
// Type                        1 byte :
// Checksum                    2 bytes:
// Data                        * byte(s)

// Packet size = 11 bytes + * bytes (data length)
class Packet {
public:

  Packet();
  
  // Read raw packet data into Packet format
  void parse(const std::string & raw_packet);
    
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

#endif // PACKET_HPP
