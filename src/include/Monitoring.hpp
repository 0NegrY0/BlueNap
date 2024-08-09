#ifndef MONITORING_HPP
#define MONITORING_HPP

#include "Utils.hpp"

#define MONITORING_PORT 42000
#define MONITORING_MESSAGE_RESPONSE "I am Awake"

using namespace std;

class Monitoring : public Utils {

public:
    int server();
    int client();
};

#endif