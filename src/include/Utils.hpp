#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>
#include <mutex>
#include <filesystem>

#define MAX_BUFFER_SIZE 1024
#define TIMEOUT_SEC 5

using namespace std;

namespace fs = filesystem;

struct Computer {
    string macAddress;
    string ipAddress;
    int id;
    bool isServer;
    bool isAwake;
};

extern vector<Computer> computers;
extern mutex mtx;

class Utils {
    public:
        string getIPAddress();
        string getMacAddress();
        string getManagerIp(const vector<Computer>& computers);
        int createSocket();
        void setSocketTimeout(int sockfd, int sec);
        struct sockaddr_in configureAdress(const string& ip, int ports);
};

#endif