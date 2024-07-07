#ifndef MONITORING_SUBSERVICE_HPP
#define MONITORING_SUBSERVICE_HPP

#include <string>
#include <vector>
#include <mutex>
#include <netinet/in.h>
#include "../utils.hpp"

using namespace std;

class MonitoringSubservice : public Utils {
private:
    Utils utils;

public:
    MonitoringSubservice();
    string getIPAddress();
    string getMacAddress();
    string getManagerIp(const vector<Computer>& computers);
    int createSocket();
    void setSocketTimeout(int sockfd, int sec);
    struct sockaddr_in configureServerAddress(const string& ip, int ports);
    int server();
    int client();
};

#endif // MONITORING_SUBSERVICE_HPP