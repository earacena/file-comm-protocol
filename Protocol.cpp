// name: emanuel aracena beriguete
// date: february 14, 2020
// filename: protocol.cpp
// description: protocol for single client/single server transmission.

#include "Protocol.hpp"

Protocol::Protocol(const bool logging) { logging_ = logging; }

std::string Protocol::str_to_hex(const std::string & unencoded) {
  static const std::string hex_digits("0123456789ABCDEF");

  std::string encoded;
  // every character is a hex pair, reserve double the size of message
  encoded.reserve(unencoded.size() * 2);

  for (unsigned char ch : unencoded) {
    // Find hex for most significant 4 bits 
    encoded.push_back(hex_digits[ch >> 4]);

    // c & 15 is same as 00001111, finding hex for least signif. 4 bits
    encoded.push_back(hex_digits[ch & 15]);
  }

  return encoded;
}

std::string Protocol::hex_to_str(const std::string & encoded) {
  size_t len = encoded.length();

  // If odd length and invalid input since it must be in hex pairs
  if (len % 2 != 0) 
    return "ERROR, invalid length";

  std::string unencoded;
  unencoded.reserve(len/2);
  for(auto it = unencoded.begin(); it != unencoded.end(); ) {
    int high_bits = hex_value(*it++);
    int low_bits = hex_value(*it++);
    // Combine into pair
    unencoded.push_back(high_bits << 4 | low_bits);
  }

  return unencoded;
}


// Private/helper functions

int Protocol::hex_value(const char hex_digit) {
  if (hex_digit >= '0' && hex_digit <= '9')
    return hex_digit - '0';

  if (hex_digit >= 'A' && hex_digit <= 'F')
    return (hex_digit - 'A') + 10;

  if (hex_digit >= 'a' && hex_digit <= 'f')
    return (hex_digit - 'a') + 10;

  // If value not returned then invalid character given
  return -1;
}
