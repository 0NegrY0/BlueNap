#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>
#include <mutex>

#define MAX_BUFFER_SIZE 1024

using namespace std;

namespace fs = filesystem;

struct Computer {
    string macAddress;
    string ipAddress;
    int id;
    bool isServer;
    bool isAwake;
};

vector<Computer> computers;
mutex mtx;

class Utils {
    public:
        string getIPAddress();
        string getMacAddress();
        string getManagerIp(const vector<Computer>& computers);
        int createSocket();
        void setSocketTimeout(int sockfd, int sec);
        struct sockaddr_in configureServerAddress(const string& ip, int ports);
};

#endif