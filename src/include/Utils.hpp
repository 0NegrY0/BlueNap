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
#define EXIT_MESSAGE "KILLME"
#define ELECTION_MESSAGE "ELECTION: "
#define ELECTION_RESPONSE "ELECTION RESPONSE"
#define ELECTION_RESULT "I AM THE NEW LEADER"
#define MONITORING_MESSAGE "Are you Awake?"

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
extern int internalClock;
extern int nextID;
extern int isMaster;

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
        int askToCloseConnection();
        bool isElectionMessage(char* buffer);
        bool isMessage(char* buffer, string message);
};

#endif