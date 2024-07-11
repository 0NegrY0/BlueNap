#ifndef INTERFACE_HPP
#define INTERFACE_HPP

#include "Utils.hpp"

using namespace std;

class Interface : public Utils {

public:
    int server();
    int client();
};

#endif 
