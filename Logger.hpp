// Name: Emanuel Aracena Beriguete
// Date: February 14, 2020
// Filename: Logger.hpp
// Description: Logger class records events in protocol for easier debugging.

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <algorithm>
#include <iostream>
#include <string>
#include <ctime>
#include <vector>
#include <fstream>

class Logger {
public:
  Logger();
  Logger(const std::string & filename);
  void set_filename(const std::string & filename);
  void record_event(const std::string & message);
  void save_log(); 

  

private:
    std::string filename_;
    std::vector<std::string> events_;
};

#endif // LOGGER_HPP
