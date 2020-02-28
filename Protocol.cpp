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
    logger.set_filename(filename);
  }
}

Protocol::~Protocol() {
  if (logging_)
    logger.save_log();
}


// ---------------------- Logging specific ---------------------
void Protocol::error(const std::string & error_msg) {
  std::cout << error_msg << std::endl;
  //logger.record_event(error_msg);
}
