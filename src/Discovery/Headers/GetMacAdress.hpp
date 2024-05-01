#ifndef GET_MAC_ADDRESS_HPP
#define GET_MAC_ADDRESS_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <filesystem>

namespace fs = std::filesystem;

std::string getMacAddress();

#endif // GET_MAC_ADDRESS_HPP
