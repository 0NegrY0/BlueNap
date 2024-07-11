#include "../include/Monitoring.hpp"
#include <iostream>
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
#include <mutex>
#include <vector>
#include <atomic>

using namespace std;

int Monitoring::server() {

    char buffer[MAX_BUFFER_SIZE];

    int ports;  //TODO

    int sockfd = createSocket();
    setSocketTimeout(sockfd, TIMEOUT_SEC);

    while (true) {
        for (int i = 0; i < computers.size(); i++) {
            struct sockaddr_in clientAddr = configureServerAddress(computers[i].ipAddress, ports);
            if (clientAddr.sin_family == AF_UNSPEC) {
                cout << "Erro ao configurar o socket" << endl;
                return -1;
            }

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
    
int Monitoring::client() {
    int sockfd = createSocket();

    int ports;  //TODO

    string managerIp = getManagerIp(computers);
    if (managerIp == "") {
        cerr << "Erro ao obter o IP do gerenciador" << endl;
        return -1;
    }

    struct sockaddr_in managerAddr = configureServerAddress(managerIp, ports);
    if (managerAddr.sin_family == AF_UNSPEC) {
        cout << "Erro ao configurar o socket" << endl;
        return -1;
    }

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


