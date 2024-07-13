#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>
#include <mutex>
#include <filesystem>

#define MAX_BUFFER_SIZE 1024
#define TIMEOUT_SEC 5
#define PORT_DISCOVERY 40000
#define TEST_PORT 40001

using namespace std;

namespace fs = filesystem;

struct Computer {
    string macAddress;
    string ipAddress;
    int id;
    bool isServer;
    bool isAwake;
    int port;
};

extern vector<Computer> computers;
extern mutex mtx;
extern string serverIp;
extern int serverPort;
extern int myPort;

class Utils {
    public:
        string getIPAddress();
        string getMacAddress();
        string getManagerIp(const vector<Computer>& computers);
        int createSocket();
        void setSocketTimeout(int sockfd, int sec);
        struct sockaddr_in configureAdress(const string& ip, int ports);
        bool isTimeoutError();
        int listenAtPort(int socket, int port);
};

#endif