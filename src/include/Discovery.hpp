#ifndef DISCOVERY_HPP
#define DISCOVERY_HPP

#include <atomic>
#include "Utils.hpp"

#define PORT_DISCOVERY 8888
#define MAX_BUFFER_SIZE 1024
#define DISCOVERY_MESSAGE "wakeywakey"

using namespace std;

class Discovery : public Utils {

public:
    atomic<bool> stopSending;
    atomic<bool> isDiscovered;
    int server();
    int client();
};

#endif