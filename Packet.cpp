// Name: Emanuel Aracena Beriguete
// Date: February 18, 2020
// Filename: Packet.cpp
// Description: A container for various packet types/formats.

#include "Packet.hpp"

Packet::Packet() {
  packet_size = 0;
  start_by = 0;
  end_by = 0;
  packet_num = 0;
  total_packets = 0;
  sender_id = "";
  receiver_id = "";
  type = "";
  checksum = "";
  data = "";
}

// Read raw packet data into Packet format
void Packet::parse(const std::string & raw_packet) { 
  Protocol p;
  packet_size = p.hex_to_dec(raw_packet.substr(0,4)); 
  start_by =  p.hex_to_dec(raw_packet.substr(4,2)); 
  end_by = p.hex_to_dec(raw_packet.substr(6,2)); 
  packet_num = p.hex_to_dec(raw_packet.substr(8,4)); 
  total_packets = p.hex_to_dec(raw_packet.substr(12,4)); 
  sender_id = raw_packet.substr(16,2); 
  receiver_id = raw_packet.substr(18,2); 
  type = raw_packet.substr(20,2); 
  checksum = raw_packet.substr(22,4); 
  data = raw_packet.substr(26, (end_by-start_by));
  
}
  
// Return Packet data in raw (hex) data for submission
std::string Packet::encode() {
  Protocol p;

  std::string p_size(p.dec_to_hex(packet_size));
  while (p_size.length() < 4)
    p_size = "0" + p_size;

  std::string start(p.dec_to_hex(start_by));
  while (start.length() < 2)
    start = "0" + start;

  std::string end(p.dec_to_hex(end_by));
  while (end.length() < 2)
    end = "0" + end;

  std::string num(p.dec_to_hex(packet_num));
  while (num.length() < 4)
    num = "0" + num;

  std::string total(p.dec_to_hex(total_packets));
  while (total.length() < 4)
    total = "0" + total;

  while (sender_id.length() < 2)
    sender_id = "0" + sender_id;

  while (receiver_id.length() < 2)
    receiver_id = "0" + receiver_id;

  while (checksum.length() < 4) 
    checksum = "0" + checksum;

   return p_size + start + end + num + total +
         sender_id + receiver_id + type + checksum + data;
}

// Calculate checksum, done in last step, returns hex value
std::string Packet::compute_checksum() {
  // Check every field, if one is not filled, return ERROR
  if (packet_size < 26 ||
    start_by == 0 || (start_by > end_by) ||
    end_by == 0 || (end_by < start_by) ||
    packet_num <= 0 || packet_num > total_packets ||
    total_packets <= 0 ||
    sender_id == "" ||
    receiver_id == "" ||
    type == "" ) {
    
    std::cout << packet_size << std::endl << start_by  << std::endl<< end_by  << std::endl<< packet_num  << std::endl<< total_packets << std::endl << sender_id  << std::endl<< receiver_id  << std::endl<< type << std::endl; 
    return "ERR";
  }

  int checksum = 0;
  Protocol protocol;
  checksum = packet_size + start_by + end_by + packet_num + total_packets;
  checksum += protocol.hex_to_dec(sender_id) + protocol.hex_to_dec(receiver_id) + 
              protocol.hex_to_dec(type);

  std::string checksum_hex = protocol.dec_to_hex(checksum);
  while (checksum_hex.length() < 4) {
    checksum_hex = "0" + checksum_hex;
  }

  std::string new_checksum_hex = checksum_hex;
  std::string carry = "";
  

  while (new_checksum_hex.length() > 4) {
    new_checksum_hex = checksum_hex.substr(checksum_hex.length()-4);
    carry = checksum_hex.substr(0, checksum_hex.length()-4);
  
    int new_checksum = protocol.hex_to_dec(new_checksum_hex);
    int carry_int = protocol.hex_to_dec(carry);
  
    new_checksum += carry_int;
    new_checksum_hex = protocol.dec_to_hex(new_checksum);
  }
  
  return new_checksum_hex;
} 


