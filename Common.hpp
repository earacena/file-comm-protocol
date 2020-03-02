// Name: Emanuel Aracena Beriguete
// Date: February 27, 2020
// Filename: Common.hpp
// Description: Implementations of commonly used utility functions.

#ifndef COMMON_HPP
#define COMMON_HPP

#include <iostream>
#include <random>
#include <sstream>
#include <ios>
// --------------------- Hex conversion helpers ----------------------
inline std::string str_to_hex(const std::string & unencoded) {
  static const std::string hex_digits("0123456789ABCDEF");

  std::string encoded;
  // every character is a hex pair, reserve double the size of message
  encoded.reserve(unencoded.size() * 2);

  for (unsigned char ch : unencoded) {
    // Find hex for most significant 4 bits 
    encoded.push_back(hex_digits.at(ch >> 4));

    // c & 15 is same as 00001111, finding hex for least signif. 4 bits
    encoded.push_back(hex_digits.at(ch & 15));
  }

  return encoded;
}

inline std::string hex_to_str(const std::string & encoded) {
  size_t len = encoded.length();

  // If odd length and invalid input since it must be in hex pairs
  if (len % 2 != 0) 
    return "ERROR, invalid length";

  std::string unencoded;
  unencoded.reserve(len/2);
  std::string byte = "";
  for(int i = 0; i < len; i += 2) {
    byte = encoded.substr(i,2);
    char ch = (char) (int)strtol(byte.c_str(), NULL, 16);
    unencoded.push_back(ch);
  }

  return unencoded;
}

inline std::string random_hex_str(int length) {
  std::string result("");
  std::string hex_digits("0123456789ABCDEF");

  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<> dist(0, 15);
  
  int rand_val = 0;
  for (int i = 0; i < length; ++i) {
    rand_val = dist(mt);
    result.append(hex_digits.substr(rand_val,1));
  }
  return result;
}

inline int random_number(int length) {
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<> dist(0, 10000);
  return dist(mt);
}

inline int hex_value(const char hex_digit) {
  if (hex_digit >= '0' && hex_digit <= '9')
    return hex_digit - '0';

  if (hex_digit >= 'A' && hex_digit <= 'F')
    return (hex_digit - 'A') + 10;

  if (hex_digit >= 'a' && hex_digit <= 'f')
    return (hex_digit - 'a') + 10;

  // If value not returned then invalid character given
  return -1;
}

inline int hex_to_dec(const std::string & encoded) {
  return std::stoi(encoded, nullptr, 16);
}

inline std::string dec_to_hex(int number) {
  std::stringstream stream;
  stream << std::hex << number;
  return stream.str();
}

#endif // COMMON_HPP
