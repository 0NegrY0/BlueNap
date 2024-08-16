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
    int sockfd = createSocket();
    setSocketTimeout(sockfd, TIMEOUT_SEC);
    listenAtPort(sockfd, myPort);
    while (isMaster) {
        
        for (size_t i = 0; i < computers.size(); i++) {
            
            if (computers[i].id == myPort - DEFAULT_PORT) {
                continue;
            }

            string clientIp = computers[i].ipAddress;
            int clientPort = computers[i].port;

            struct sockaddr_in clientAddr = configureAdress(clientIp, clientPort);
            socklen_t clientLen = sizeof(clientAddr);

            if (computers[i].ipAddress == oldServerIP) {
                strcpy(buffer, NEW_LEADER_MESSAGE);
                sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&clientAddr, clientLen);
                clientAddr = configureAdress(clientIp, clientPort);
                memset(buffer, 0, MAX_BUFFER_SIZE);
                int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&clientAddr, &clientLen);
                if (bytesReceived > 0) {
                    cout << "Recebi alguma coisa:" << buffer << endl;
                    if (strcmp(buffer, OLD_LEADER_RESPONSE) == 0) {
                        strcpy(buffer, OK);
                        sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&clientAddr, clientLen);
                        mtx.lock();
                        oldServerIP = "";
                        internalClock += 1;
                        mtx.unlock();
                    }
                }
            }
            else {
                vector<char> send;
                send = management.setMonitoringMessage();
                //Ensure the vector is null-terminated if necessary
                if (send.empty() || send.back() != '\0') {
                    send.push_back('\0');
                }

                sendto(sockfd, send.data(), send.size(), 0, (struct sockaddr*)&clientAddr, clientLen);
            }

            clientAddr = configureAdress(clientIp, clientPort);

            memset(buffer, 0, MAX_BUFFER_SIZE);

            int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&clientAddr, &clientLen);
            if (bytesReceived < 0) {
                if (isTimeoutError()) {
                    management.updateStatus(computers[i].id, false);
                }
                else {
                    cerr << "Error in recvfrom(): " << "erro monitoring" << strerror(errno) << endl;
                    do {
                        cout << "Enviei" << endl;
                        bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&clientAddr, &clientLen);
                    } while (bytesReceived < 0 && errno == EINTR); 
                    cout << "Vou Fechar" << endl;
                    continue;
                }
            }
            else {
                buffer[bytesReceived] = '\0'; // Adiciona um terminador nulo para evitar problemas com a comparação
                if (strcmp(buffer, MONITORING_MESSAGE_RESPONSE) == 0) {
                    management.updateStatus(computers[i].id, true);
                }
                
                // SERVER NOVO: SOU O NOVO LIDER 
                // SERVER ANTIGO: OK SORRY
                // SERVER NOVO: OK
                if (isMessage(buffer, NEW_LEADER_MESSAGE)) {
                    bool exit = false;
                    do {
                        setSocketTimeout(sockfd, 0.5);
                        strcpy(buffer, OLD_LEADER_RESPONSE);
                        sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&clientAddr, clientLen);
                        clientAddr = configureAdress(clientIp, clientPort);
                        memset(buffer, 0, MAX_BUFFER_SIZE);
                        int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&clientAddr, &clientLen);
                        if (bytesReceived > 0) {
                            if (strcmp(buffer, OK) == 0) {
                                exit = true;
                            }
                        }
                    }while(!exit);
                    cout << "Mestre me ouviu" << endl;
                    setSocketTimeout(sockfd, TIMEOUT_SEC);
                    sleep(2);
                    mtx.lock();
                    isMaster = 0;
                    oldServerIP = "";
                    mtx.unlock();
                    cout << "isMaster: " << isMaster << endl;
                }
            }
        }
        cout << "sai do for";
        cout << "IsMaster fora do for: " << isMaster << endl;
        sleep(1);
    }
    cout << "sai do while do master";
    close(sockfd);
    cout << "fechei o socket";
    return 0;
}
    
int Monitoring::client() {
    cout << "Monitor cliente" << endl;
    while (serverIp.empty());

    int sockfd = createSocket(); // recriar o socket se der erro.
    setSocketTimeout(sockfd, 7);

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

    char buffer[MAX_BUFFER_SIZE];
    struct sockaddr_in serverAddr; // = configureAdress(serverIp, serverPort);
    socklen_t serverLen = sizeof(serverAddr);
    Management management;
    
    while(!shouldExit && !isMaster) {
        int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&serverAddr, &serverLen);
        
        if (bytesReceived < 0) {
            if (isTimeoutError()) {
                management.startElection(myPort - DEFAULT_PORT, sockfd);
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

        if (isElectionMessage(buffer)) {
            string message(buffer);
            cout << "Recebi mensagem de eleicao:" << buffer << endl;

            size_t maxIdPos = message.find(ELECTION_MESSAGE);

            if (maxIdPos == string::npos) {
                cerr << "Invalid election message format" << endl;
                continue;
            }
            int id = stoi(message.substr(maxIdPos + strlen(ELECTION_MESSAGE)));
            int myId = myPort - DEFAULT_PORT;
            if (myId < id) {
                string response = "RESPONSE" + to_string(myId);
                char* responseMessage = new char[MAX_BUFFER_SIZE];
                snprintf(responseMessage, MAX_BUFFER_SIZE, "%s", response.c_str());
                cout << "Meu id é menor, vou chamar uma eleiçao: " << responseMessage << endl;
                sendto(sockfd, responseMessage, strlen(responseMessage), 0, (struct sockaddr*)&serverAddr, serverLen);
                sleep(2);
                management.startElection(myId, sockfd);
            }
        } 

        if (isMessage(buffer, ELECTION_RESULT)) {
            cout << "Recebi mensagem de resultado de eleição" << endl;
            string message(buffer);
            cout << "Mensagem: " << message << endl;

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
            serverPort = ntohs(serverAddr.sin_port);
            serverHostName = hostName;
            serverMac = hostMac;
            mtx.unlock();
        }
    }
    close(sockfd);
    return 0;
}