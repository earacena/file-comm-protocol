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
  packet_size = hex_to_dec(raw_packet.substr(0,4)); 
  start_by =  hex_to_dec(raw_packet.substr(4,2)); 
  end_by = hex_to_dec(raw_packet.substr(6,2)); 
  packet_num = hex_to_dec(raw_packet.substr(8,4)); 
  total_packets = hex_to_dec(raw_packet.substr(12,4)); 
  sender_id = raw_packet.substr(16,2); 
  receiver_id = raw_packet.substr(18,2); 
  type = raw_packet.substr(20,2); 
  checksum = raw_packet.substr(22,4); 
  data = raw_packet.substr(26);

}


// Virtual function implementations

void SynPacket::craft(Protocol & proto, const std::string & payload) {
  proto.session_id = random_hex_str(2);
  sender_id = proto.session_id;

  std::string receiver_session_id(random_hex_str(2));
  receiver_id = receiver_session_id;
  type = "00";

  proto.sequence_number = random_number(4);
  std::string hex_seq_num(dec_to_hex(proto.sequence_number));
  if (hex_seq_num.length() < 4)
    hex_seq_num = "0" + hex_seq_num;

  data = hex_seq_num;
  start_by = 26;
  end_by = start_by + data.length();
  packet_size = 26 + data.length();
  packet_num = 1;
  total_packets = 1;
  checksum = compute_checksum();

  std::string action = "crafted";
  //proto.logger.record_event(*this, action); 

}


void SynAckPacket::craft(Protocol & proto, const std::string & payload) {
  // Extract useful info from payload
  // receiver id, payload(0,3)
  // sender id, session id
  sender_id = proto.session_id;
  receiver_id = payload.substr(0,2);

  // Packet type
  type = "01";

  // Increment sequence number x
  int sequence_number_x = hex_to_dec(payload.substr(2,4));

  sequence_number_x += 1;
  std::string hex_seq_num_x(dec_to_hex(sequence_number_x));
  if (hex_seq_num_x.length() < 4)
    hex_seq_num_x = "0" + hex_seq_num_x;
  data.append(hex_seq_num_x);

  // Generate sequence number y
  proto.sequence_number = random_number(4);
  std::string hex_seq_num_y(dec_to_hex(proto.sequence_number));
  if (hex_seq_num_y.length() < 4)
    hex_seq_num_y = "0" + hex_seq_num_y;
  data.append(hex_seq_num_y);

  packet_size = 26 + data.length();
  start_by = 26;
  end_by = data.length() + start_by;
  packet_num = 1;
  total_packets = 1;
  checksum = compute_checksum();


  //proto.logger.record_event(*this, "crafted");
}


void AckPacket::craft(Protocol & proto, const std::string & payload) {

  sender_id = proto.session_id;
  receiver_id = payload.substr(0,2);

  // Packet type
  type = "02";

  // Generate sequence number y
  std::string hex_seq_num_y(dec_to_hex(proto.sequence_number_y + 1));
  if (hex_seq_num_y.length() < 4)
    hex_seq_num_y = "0" + hex_seq_num_y;
  data.append(hex_seq_num_y);

  packet_size = 26 + data.length();
  start_by = 26;
  end_by = start_by + data.length();
  packet_num = 1;
  total_packets = 1;
  checksum = compute_checksum();

  //proto.logger.record_event(*this, "crafted");

}

void BufferRequestPacket::craft(Protocol & proto, const std::string & payload) {
  sender_id = proto.session_id;
  receiver_id = payload.substr(0,2);
  packet_size = 26;
  start_by = 26;
  end_by = 26;
  packet_num = 1;
  total_packets = 1;
  type = "07";
  checksum = compute_checksum();
  data = "";

  //proto.logger.record_event(*this, "crafted");

}


void BufferResponsePacket::craft(Protocol & proto, const std::string & payload) {
  sender_id = proto.session_id;
  receiver_id = payload.substr(0,2);
  data = payload;
  packet_size = 26 + payload.length() - 2;
  start_by = 26;
  end_by = start_by + data.length();
  packet_num = 1;
  total_packets = 1;
  type = "08";
  checksum = compute_checksum();

  //proto.logger.record_event(*this, "crafted");

}
 
void DataPacket::craft(Protocol & proto, const std::string & payload) {

  sender_id = proto.session_id;
  
  receiver_id = payload.substr(0,2);

  std::string encoded = str_to_hex(payload.substr(2));
  data = encoded;
  packet_size = 26 + encoded.length();
  start_by = 26;
  end_by = start_by + data.length();
  if (end_by > 255) {
    // Largest possible message size is 229 until reaching pre-transmit phase
    end_by = 255;
    data = data.substr(0,229);
  }
  packet_num = 1;
  total_packets = 1;
  type = "09";
  checksum = compute_checksum();

  //proto.logger.record_event(*this, "crafted");
}
 
 
// Return Packet data in raw (hex) data for submission
std::string Packet::encode() {

  std::string p_size(dec_to_hex(packet_size));
  while (p_size.length() < 4)
    p_size = "0" + p_size;

  std::string start(dec_to_hex(start_by));
  while (start.length() < 2)
    start = "0" + start;

  std::string end(dec_to_hex(end_by));
  while (end.length() < 2)
    end = "0" + end;

  std::string num(dec_to_hex(packet_num));
  while (num.length() < 4)
    num = "0" + num;

  std::string total(dec_to_hex(total_packets));
  while (total.length() < 4)
    total = "0" + total;

  while (sender_id.length() < 2)
    sender_id = "0" + sender_id;

  while (receiver_id.length() < 2)
    receiver_id = "0" + receiver_id;

  while (checksum.length() < 4) 
    checksum = "0" + checksum;

   std::string encoded = p_size + start + end + num + total + sender_id +
                         receiver_id + type + checksum + data;

  // Make encoding uniform
  std::transform(encoded.begin(), encoded.end(), encoded.begin(), ::toupper);
  
  return encoded;
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
  checksum = packet_size + start_by + end_by + packet_num + total_packets;
  checksum += hex_to_dec(sender_id) + hex_to_dec(receiver_id) + 
              hex_to_dec(type);

  std::string checksum_hex = dec_to_hex(checksum);
  while (checksum_hex.length() < 4) {
    checksum_hex = "0" + checksum_hex;
  }

  std::string new_checksum_hex = checksum_hex;
  std::string carry = "";
  

  while (new_checksum_hex.length() > 4) {
    new_checksum_hex = checksum_hex.substr(checksum_hex.length()-4);
    carry = checksum_hex.substr(0, checksum_hex.length()-4);
  
    int new_checksum = hex_to_dec(new_checksum_hex);
    int carry_int = hex_to_dec(carry);
  
    new_checksum += carry_int;
    new_checksum_hex = dec_to_hex(new_checksum);
  }
 
  std::transform(new_checksum_hex.begin(), new_checksum_hex.end(), new_checksum_hex.begin(), ::toupper);
 
  return new_checksum_hex;
} 


