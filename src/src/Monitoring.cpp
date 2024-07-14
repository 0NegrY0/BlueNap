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
        for (size_t i = 1; i < computers.size(); i++) {
            int sockfd = createSocket();
            setSocketTimeout(sockfd, TIMEOUT_SEC);

            string clientIp = computers[i].ipAddress;
            int clientPort = computers[i].port;

            struct sockaddr_in clientAddr = configureAdress(clientIp, clientPort);
            socklen_t clientLen = sizeof(clientAddr);

            strcpy(buffer, MONITORING_MESSAGE);

            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&clientAddr, clientLen);

            clientAddr = configureAdress(clientIp, clientPort);

            int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&clientAddr, &clientLen);
            if (bytesReceived < 0) {
                if (isTimeoutError()) {
                    updateStatus(computers[i].id, false);
                }
                else {
                    cerr << "Error in recvfrom(): " << strerror(errno) << endl;
                    close(sockfd);
                    continue;
                }
            }
            else {
                buffer[bytesReceived] = '\0'; // Adiciona um terminador nulo para evitar problemas com a comparação
                if (strcmp(buffer, MONITORING_MESSAGE_RESPONSE) == 0) {
                    updateStatus(computers[i].id, true);
                }
            }
            close(sockfd);
        }
        sleep(5);
    }
    return 0;
}
    
int Monitoring::client() {

    while (serverIp.empty());

    int sockfd = createSocket();

    //struct sockaddr_in localAddr = configureAddress(serverIp, myPort);
    struct sockaddr_in localAddr;
    socklen_t localLen = sizeof(localAddr);

    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = INADDR_ANY;
    localAddr.sin_port = htons(myPort);

    if (bind(sockfd, (struct sockaddr*)&localAddr, localLen) < 0) {
        cerr << "Error in bind(): " << strerror(errno) << endl;
        close(sockfd);
        return -1;
    }

    struct sockaddr_in serverAddr = configureAdress(serverIp, PORT_DISCOVERY);
    socklen_t serverLen = sizeof(serverAddr);

    char buffer[MAX_BUFFER_SIZE];
    
    while(!shouldExit) {
        int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&serverAddr, &serverLen);
        if (bytesReceived < 0) {
            std::cerr << "Error in recvfrom(): " << strerror(errno) << endl;
            continue;
        }

        buffer[bytesReceived] = '\0';
        if (strcmp(buffer, MONITORING_MESSAGE) == 0) {
            strcpy(buffer, MONITORING_MESSAGE_RESPONSE);
            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&serverAddr, serverLen);
        }
    }
    close(sockfd);
    return 0;
}