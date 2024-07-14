#ifndef MANAGEMENT_HPP
#define MANAGEMENT_HPP

#include <vector>
#include <mutex>
#include <filesystem>
#include <string>

#define MAX_BUFFER_SIZE 1024
#define TIMEOUT_SEC 5
#define PORT_DISCOVERY 40000
#define TEST_PORT 40001
#define EXIT_MESSAGE "KILLME"

using namespace std;

namespace fs = filesystem;

struct Computer {
    string macAddress;
    string ipAddress;
    string hostName;
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
extern string serverHostName;
extern string serverMac;
extern bool shouldExit;

class Management {

public:
    void addComputer(const Computer& computer);
    void removeComputer(const string& ip);
    void updateStatus(int id, bool isAwake);
    vector<Computer> getComputers();
    void wakeOnLan(const string& macAddress, const string& ipAddress);
    void handleStatusUpdate(int id, bool isAwake);
    string getIPAddress();
    string getMacAddress();
    string getManagerIp(const vector<Computer>& computers);
    int createSocket();
    void setSocketTimeout(int sockfd, int sec);
    struct sockaddr_in configureAdress(const string& ip, int ports);
    bool isTimeoutError();
    int listenAtPort(int socket, int port);
    int askToCloseConnection();
};

#endif