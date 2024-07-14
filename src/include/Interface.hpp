#ifndef INTERFACE_HPP
#define INTERFACE_HPP

#include "Management.hpp"

using namespace std;

class Interface : public Management {

public:
    int server();
    int client();
};

#endif 
