#ifndef MONITORING_HPP
#define MONITORING_HPP

#include "Utils.hpp"

#define MONITORING_PORT 42000
#define MONITORING_MESSAGE "Are you Awake?"
#define MONITORING_MESSAGE_RESPONSE "I am Awake"
#define TIMEOUT_SEC 5

using namespace std;

class Monitoring : public Utils {

public:
    int server();
    int client();
};

#endif