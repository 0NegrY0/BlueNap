#include "Monitoring_Subservice.hpp"
#include <iostream>
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
#include <mutex>
#include <vector>
#include <atomic>

#define MONITORING_PORT 42000
#define MONITORING_MESSAGE "Are you Awake?"
#define MONITORING_MESSAGE_RESPONSE "I am Awake"
#define TIMEOUT_SEC 5

using namespace std;

// mutex mtx;

int MonitoringSubservice::server() {

    char buffer[MAX_BUFFER_SIZE];

    int sockfd = createSocket();
    setSocketTimeout(sockfd, TIMEOUT_SEC);

    while (true) {
        for (int i = 0; i < computers.size(); i++) {
            struct sockaddr_in clientAddr = Utils::configureServerAddress(computers[i].ipAddress);

            strcpy(buffer, MONITORING_MESSAGE);

            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));

            int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&clientAddr, (socklen_t*)sizeof(clientAddr));
            if (bytesReceived < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) { //PC ta dormindo
                    mtx.lock();
                    computers[i].isAwake = false;
                    mtx.unlock();
                }
                else {
                    std::cerr << "Error in recvfrom()" << std::endl;
                    continue;
                }
            }
            else {
                if (strcmp(buffer, MONITORING_MESSAGE_RESPONSE) == 0) {
                    mtx.lock();
                    computers[i].isAwake = true;
                    mtx.unlock();
                }
            }
        }
    }

    close(sockfd);
}
    
int MonitoringSubservice::client() {
    int sockfd = createSocket();

    string managerIp = getManagerIp(computers);
    if (managerIp == "") {
        cerr << "Erro ao obter o IP do gerenciador" << endl;
        return -1;
    }

    struct sockaddr_in managerAddr = configureServerAddress(managerIp);

    char buffer[MAX_BUFFER_SIZE];

    while (true) {
        int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&managerAddr, (socklen_t*)sizeof(managerAddr));
        if (bytesReceived < 0) {
            std::cerr << "Error in recvfrom()" << std::endl;
            continue;
        }
        else {
            if (strcmp(buffer, MONITORING_MESSAGE) == 0) {
                strcpy(buffer, MONITORING_MESSAGE_RESPONSE);
                sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&managerAddr, sizeof(managerAddr));
            }
        }
    }
}

string MonitoringSubservice::getIPAddress() {
    Utils::getIPAddress();
}

string MonitoringSubservice::getMacAddress() {
    Utils::getMacAddress();
}

string MonitoringSubservice::getManagerIp(const vector<Utils::Computer>& computers) {
    Utils::getManagerIp(computers);
}

int MonitoringSubservice::createSocket() {
    Utils::createSocket();
}

void MonitoringSubservice::setSocketTimeout(int sockfd, int sec) {
    Utils::setSocketTimeout(sockfd, sec);
}

struct MonitoringSubservice::sockaddr_in configureServerAddress(const string& ip, int ports) {
    Utils::configureServerAddress(ip, ports);
}


