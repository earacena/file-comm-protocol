// Name: Emanuel Aracena Beriguete
// Date: February 14, 2020
// Filename: Logger.cpp
// Description: Logger class records events in protocol for easier debugging.

#include "Logger.hpp"

Logger::Logger() {  }

Logger::Logger(const std::string & filename) { filename_ = filename; }
 
void Logger::set_filename(const std::string & filename) { filename_ = filename; }

void Logger::record_event(const std::string & event) {
  std::time_t result = std::time(nullptr);
  std::string time(std::asctime(std::localtime(&result)));
  time.erase(std::remove(time.begin(), time.end(), '\n'), time.end());

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

