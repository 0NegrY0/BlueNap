#ifndef AUXILIAR_FUNCTIONS_HPP
#define AUXILIAR_FUNCTIONS_HPP

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <filesystem>

namespace fs = std::filesystem;

std::string getIPAddress();

std::string getMacAddress();

#endif // GET_MAC_ADDRESS_HPP
