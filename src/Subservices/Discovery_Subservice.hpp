#ifndef DISCOVERY_SUBSERVICE_HPP
#define DISCOVERY_SUBSERVICE_HPP

#include <string>
#include <atomic>
#include <netinet/in.h>
#include "../utils.hpp"

#define PORT_DISCOVERY 8888
#define MAX_BUFFER_SIZE 1024
#define DISCOVERY_MESSAGE "wakeywakey"

using namespace std;

class DiscoverySubservice : public Utils {
private:
    atomic<bool> stopSending;
    atomic<bool> isDiscovered;

public:
    DiscoverySubservice(); 
    string getIPAddress();
    string getMacAddress();
    string getManagerIp(const vector<Computer>& computers);
    int createSocket();
    void setSocketTimeout(int sockfd, int sec);
    struct sockaddr_in configureServerAddress(const string& ip, int ports);
    int server() const override;
    int client() const override;
};

#endif // DISCOVERY_SUBSERVICE_HPP