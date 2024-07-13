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
    
    while (true) {
        for (int i = 1; i < computers.size(); i++) {
            int sockfd = createSocket();

            struct sockaddr_in clientAddr = configureAdress(computers[i].ipAddress, PORT_DISCOVERY);
            if (clientAddr.sin_family == AF_UNSPEC) {
                cout << "Erro ao configurar o socket" << endl;
                return -1;
            }

            setSocketTimeout(sockfd, TIMEOUT_SEC);

            strcpy(buffer, MONITORING_MESSAGE);

            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));

            int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&clientAddr, (socklen_t*)sizeof(clientAddr));
            if (bytesReceived < 0) {
                if (isTimeoutError()) {
                    mtx.lock();
                    computers[i].isAwake = false;
                    mtx.unlock();
                }
                else {
                    std::cerr << "Error in recvfrom()" << std::endl;
                    close(sockfd);
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
            close(sockfd);
        }
    }
}
    
int Monitoring::client() {
    while (serverIp.empty());
    
    int sockfd = createSocket();
    struct sockaddr_in serverAddr = configureAdress(serverIp, serverPort);

    if (serverAddr.sin_family == AF_UNSPEC) {
        cout << "Erro ao configurar o socket" << endl;
        return -1;
    }

    char buffer[MAX_BUFFER_SIZE];
    
    while (true) {
        int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&serverAddr, (socklen_t*)sizeof(serverAddr));
        if (bytesReceived < 0) {
            std::cerr << "Error in recvfrom()" << std::endl;
            continue;
        }

        if (strcmp(buffer, MONITORING_MESSAGE) == 0) {
            strcpy(buffer, MONITORING_MESSAGE_RESPONSE);
            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        }
        
    }
}