#ifndef DISCOVERY_HPP
#define DISCOVERY_HPP

#include <atomic>
#include "Management.hpp"

#define MAX_BUFFER_SIZE 1024
#define DISCOVERY_MESSAGE "wakeywakey"
#define DISCOVERY_RESPONSE "Discovered"
#define BROADCAST_IP "255.255.255.255"

using namespace std;

class Discovery : public Management {

public:
    atomic<bool> stopSending;
    atomic<bool> isDiscovered;
    int server();
    int client();
    bool isDiscoveryMessage(char* buffer);
    bool isDiscoveryResponse(char* buffer);
    bool isExitMessage(char* buffer);
    char* setDiscoveryResponse(int clientPort);
    struct Computer createComputer(string clientIp, string clienMac);
    int isAlreadyDiscovered(string clientIp);
};

#endif