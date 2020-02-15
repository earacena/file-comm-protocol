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

  private:
    // Allow logger to record protocol events
    friend class Logger;
    bool logging_;

    int hex_value(const char hex_digit);

};

#endif // PROTOCOL_HPP
