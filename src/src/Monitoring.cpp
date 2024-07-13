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
            struct sockaddr_in clientAddr;
            socklen_t addrLen = sizeof(clientAddr);
            memset(&clientAddr, 0, sizeof(clientAddr));
            clientAddr.sin_family = AF_INET;
            clientAddr.sin_port = htons(computers[i].port);
            if (inet_pton(AF_INET, computers[i].ipAddress.c_str(), &clientAddr.sin_addr) <= 0) {
                std::cerr << "Endereço inválido/Não suportado." << std::endl;
                close(sockfd);
                exit(EXIT_FAILURE);
            }

            setSocketTimeout(sockfd, TIMEOUT_SEC);

            strcpy(buffer, MONITORING_MESSAGE);

            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&clientAddr, addrLen);

            struct sockaddr_in fromAddr;
            socklen_t fromAddrLen = sizeof(fromAddr);
            int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&fromAddr, &fromAddrLen);
            if (bytesReceived < 0) {
                if (isTimeoutError()) {
                    mtx.lock();
                    computers[i].isAwake = false;
                    mtx.unlock();
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
                    mtx.lock();
                    computers[i].isAwake = true;
                    mtx.unlock();
                }
            }
            close(sockfd);
        }
        sleep(5);
    }
    return 0; // Adiciona retorno para evitar warnings de compilação
}
    
int Monitoring::client() {

    while (serverIp.empty());

    int sockfd = createSocket();

    struct sockaddr_in localAddr;
    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = INADDR_ANY;
    localAddr.sin_port = htons(myPort);

    if (bind(sockfd, (struct sockaddr*)&localAddr, sizeof(localAddr)) < 0) {
        cerr << "Error in bind(): " << strerror(errno) << endl;
        close(sockfd);
        return -1;
    }

    struct sockaddr_in serverAddr;
    socklen_t serverLen = sizeof(serverAddr);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_DISCOVERY);
    if (inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr) <= 0) {
        std::cerr << "Endereço inválido/Não suportado." << std::endl;
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_BUFFER_SIZE];
    
    while (true) {
        int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&serverAddr, &serverLen);
        if (bytesReceived < 0) {
            std::cerr << "Error in recvfrom(): " << strerror(errno) << endl;
            continue;
        }

        buffer[bytesReceived] = '\0'; // Adiciona um terminador nulo para evitar problemas com a comparação
        if (strcmp(buffer, MONITORING_MESSAGE) == 0) {
            strcpy(buffer, MONITORING_MESSAGE_RESPONSE);
            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&serverAddr, serverLen);
        }
    }
    close(sockfd); // Adiciona fechamento do socket ao final do loop
    return 0; // Adiciona retorno para evitar warnings de compilação
}
