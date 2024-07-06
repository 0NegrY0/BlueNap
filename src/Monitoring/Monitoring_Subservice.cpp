#include <iostream>
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
#include <mutex>
#include <vector>
#include <atomic>
#include "src/Monitoring/Headers/Monitoring_Subservice.hpp"

using namespace std;

mutex mtx;

void handleMonitoringReceiver() {
    int sockfd = createSocket();

    string managerIp = getManagerIp();
    if (managerIp == "") {
        cerr << "Erro ao obter o IP do gerenciador" << endl;
        return -1;
    }

    struct sockaddr_in managerAddr = configureServerAddress(managerIp);

    char buffer[MAX_BUFFER_SIZE];

    while (true) {
        int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&managerAddr, sizeof(managerAddr));
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

void handleMonitoringSender() {
    char buffer[MAX_BUFFER_SIZE];

    int sockfd = createSocket();
    setSocketTimeout(sockfd, TIMEOUT_SEC);

    while (true) {
        for (int i = 0; i < computers.size(); i++) {
            struct sockaddr_in clientAddr = configureServerAddress(computers[i].ipAddress);

            strcpy(buffer, MONITORING_MESSAGE);
            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));

            int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
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

    // Fecha o socket
    close(sockfd);
}

int main() {
    vector<thread> threads;

    // Adiciona threads para lidar com a descoberta de estações participantes
    threads.push_back(thread(handleDiscoveryReceiver));

    // Adiciona threads para enviar mensagens de descoberta
    threads.push_back(thread(handleDiscoverySender));

    // Aguarda a finalização de todas as threads
    for (auto& th : threads) {
        th.join();
    }

    return 0;
}