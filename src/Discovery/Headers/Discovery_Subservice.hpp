#ifndef NETWORK_DISCOVERY_HPP
#define NETWORK_DISCOVERY_HPP

#include <iostream>
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
#include <mutex>
#include <vector>
#include <atomic>

#define PORT_DISCOVERY 8888
#define MAX_BUFFER_SIZE 1024
#define DISCOVERY_MESSAGE "sleep service discovery"

struct Computer {
    std::string macAddress;
    std::string ipAddress;
    int id;
    bool isServer;
};

extern std::mutex mtx;
extern std::atomic<bool> stopSending;
extern std::atomic<bool> isDiscovered;
extern std::vector<Computer> computers;

void handleDiscoveryReceiver();
void handleDiscoverySender();

#endif // NETWORK_DISCOVERY_HPP
