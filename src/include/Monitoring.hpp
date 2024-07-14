#ifndef MONITORING_HPP
#define MONITORING_HPP

#include "Management.hpp"

#define MONITORING_PORT 42000
#define MONITORING_MESSAGE "Are you Awake?"
#define MONITORING_MESSAGE_RESPONSE "I am Awake"

using namespace std;

class Monitoring : public Management {

public:
    int server();
    int client();
};

#endif