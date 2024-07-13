#ifndef DISCOVERY_HPP
#define DISCOVERY_HPP

#include <atomic>
#include "Utils.hpp"

#define MAX_BUFFER_SIZE 1024
#define DISCOVERY_MESSAGE "wakeywakey"
#define DISCOVERY_RESPONSE "Discovered"
#define BROADCAST_IP "255.255.255.255"

using namespace std;

class Discovery : public Utils {

public:
    atomic<bool> stopSending;
    atomic<bool> isDiscovered;
    int server();
    int client();
    bool isDiscoveryMessage(char* buffer);
    bool isDiscoveryResponse(char* buffer);
};

#endif