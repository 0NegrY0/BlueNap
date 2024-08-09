#include "../include/Monitoring.hpp"
#include "../include/Management.hpp"
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
    Management management;
    while (isMaster) {
        for (size_t i = 1; i < computers.size(); i++) {
            int sockfd = createSocket();
            setSocketTimeout(sockfd, TIMEOUT_SEC);

            string clientIp = computers[i].ipAddress;
            int clientPort = computers[i].port;

            struct sockaddr_in clientAddr = configureAdress(clientIp, clientPort);
            socklen_t clientLen = sizeof(clientAddr);

            vector<char> send = management.setMonitoringMessage();

            // // Ensure the vector is null-terminated if necessary
            // if (send.empty() || send.back() != '\0') {
            //     send.push_back('\0');
            // }

            sendto(sockfd, send.data(), send.size(), 0, (struct sockaddr*)&clientAddr, clientLen);

            clientAddr = configureAdress(clientIp, clientPort);

            int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&clientAddr, &clientLen);
            if (bytesReceived < 0) {
                if (isTimeoutError()) {
                    management.updateStatus(computers[i].id, false);
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
                    management.updateStatus(computers[i].id, true);
                }
                if (isMessage(buffer, MONITORING_MESSAGE)) {
                    management.receiveComputers(buffer);
                    if (internalClock < message.internalclock){
                            isMaster = false;
                        }
                    
                }
            }
            close(sockfd);
        }
    }
    return 0;
}
    
int Monitoring::client() {

    while (serverIp.empty());

    int sockfd = createSocket();
    setSocketTimeout(sockfd, 10);

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
    Management management;
    
    while(!shouldExit && !isMaster) {
        int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&serverAddr, &serverLen);

        if (bytesReceived < 0) {
            if (isTimeoutError()) {
                management.startElection(myPort - PORT_DISCOVERY);
            }
            else {
                std::cerr << "Error in recvfrom(): " << strerror(errno) << endl;
            }
            continue;
        }

        buffer[bytesReceived] = '\0';
        if (isMessage(buffer, MONITORING_MESSAGE)) {
            management.receiveComputers(buffer);            //TODO: Implementar a função receiveComputers
            strcpy(buffer, MONITORING_MESSAGE_RESPONSE);
            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&serverAddr, serverLen);
        }

        else if (isElectionMessage(buffer)) {
            string message(buffer);

            size_t maxIdPos = message.find(ELECTION_MESSAGE);

            if (maxIdPos == string::npos) {
                cerr << "Invalid election message format" << endl;
                continue;
            }
            int id = stoi(message.substr(maxIdPos + strlen(ELECTION_MESSAGE)));
            int myId = myPort - PORT_DISCOVERY;
            if (myId < id) {
                string response = "RESPONSE" + to_string(myId);
                char* responseMessage = new char[MAX_BUFFER_SIZE];
                snprintf(responseMessage, MAX_BUFFER_SIZE, "%s", response.c_str());
                sendto(sockfd, responseMessage, strlen(responseMessage), 0, (struct sockaddr*)&serverAddr, serverLen);
                sleep(2);
                management.startElection(myPort - PORT_DISCOVERY);
            }
        } 

        else if (isMessage(buffer, ELECTION_RESULT)) {
            string message(buffer);

            size_t hostNamePos = message.find("Host Name:");
            size_t macPos = message.find("Host Mac:");

            if (hostNamePos == string::npos || macPos == string::npos) {
                cerr << "Invalid discovery message format" << endl;
                continue;
            }

            hostNamePos = hostNamePos + strlen("Host Name:");

            string hostName = message.substr(hostNamePos, macPos - hostNamePos);

            macPos = macPos + strlen("Host Mac:");

            string hostMac = message.substr(macPos);

            mtx.lock();
            serverIp = inet_ntoa(serverAddr.sin_addr);
            //serverPort = ntohs(serverAddr.sin_port);
            serverHostName = hostName;
            serverMac = hostMac;
            mtx.unlock();
        }
    }
    close(sockfd);
    return 0;
}