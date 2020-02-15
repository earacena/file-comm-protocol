// Name: Emanuel Aracena Beriguete
// Date: February 14, 2020
// Filename: Logger.hpp
// Description: Logger class records events in protocol for easier debugging.

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>

class Logger {
  Logger(const std::string & filename);
  
  void record_event(const std::string & message);
  void save_log(); 

  private:
    std::string filename_;
};

#endif // LOGGER_HPP
