// Name: Emanuel Aracena Beriguete
// Date: February 14, 2020
// Filename: Logger.cpp
// Description: Logger class records events in protocol for easier debugging.

#include "Logger.hpp"

Logger::Logger() {  }

Logger::Logger(const std::string & filename) { filename_ = filename; }
 
void Logger::set_filename(const std::string & filename) { filename_ = filename; }

void Logger::record_event(Packet & packet, const std::string & action) {
  std::time_t result = std::time(nullptr);
  std::string time(std::asctime(std::localtime(&result)));
  time.erase(std::remove(time.begin(), time.end(), '\n'), time.end());

  std::string event("\n");

  Protocol proto;

  event = event + "Packet " + action  + " [" + packet.type + "]::\n";
  event = event + "\t| Raw Packet: " + packet.encode() + "\n";
  event = event + "\t| Packet size:\t\t" + proto.dec_to_hex(packet.packet_size) + "\n";
  event = event + "\t| Start-by:\t\t" + proto.dec_to_hex(packet.start_by) + "\n";
  event = event + "\t| End-by:\t\t" + proto.dec_to_hex(packet.end_by) + "\n";
  event = event + "\t| Packet # (out of n):\t" + proto.dec_to_hex(packet.packet_num) + "\n";
  event = event + "\t| Total packets (n):\t" + proto.dec_to_hex(packet.total_packets) + "\n";
  event = event + "\t| Session id (Sender):\t" + packet.sender_id + "\n";
  event = event + "\t| Receiver id:\t\t" + packet.receiver_id + "\n";
  event = event + "\t| Packet type:\t\t" + packet.type + "\n";
  event = event + "\t| Checksum:\t\t" + packet.checksum + "\n";
  event = event + "\t| Data:" + packet.data + "\n";
   
  events_.push_back("["+ time +"] " + event + "\n");

}

void Logger::save_log() {
  std::ofstream file(filename_);
  if (!file.is_open()) {
    std::cout << "[-] Error: Failure while creating file \"" + filename_ +
                 "\" for logging..." << std::endl;
  } else {

    file << "\n---------------- " + filename_ + " --------------\n";  

    for (std::string & event : events_)
      file << event;
  }
}

