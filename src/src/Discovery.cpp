#include "../include/Discovery.hpp"
#include <iostream>
#include <cstring>
#include <cerrno>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

int Discovery::server() {
    struct sockaddr_in serverAddr, clientAddr;

    socklen_t clientLen = sizeof(clientAddr);

    char buffer[MAX_BUFFER_SIZE];

    int sockfd = createSocket();

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT_DISCOVERY);

    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Error in bind()" << endl;
        close(sockfd);
        return -1;
    }

    cout << "Discovery service listening on port " << PORT_DISCOVERY << endl;

    while (true) {
        memset(buffer, 0, sizeof(buffer));

        int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&clientAddr, &clientLen);
        if (bytesReceived < 0) {
            cerr << "Error in recvfrom()" << endl;
            continue;
        }

        if (isDiscoveryMessage(buffer)) {
            cout << "Received discovery message from: " << inet_ntoa(clientAddr.sin_addr) << endl;

            string message(buffer);
            size_t macPos = message.find("MAC- ");

            if (macPos == string::npos) {
                cerr << "Invalid discovery message format" << endl;
                continue;
            }
                
            string mac = message.substr(macPos + 5, 17);

            Computer comp;
            comp.macAddress = mac;
            comp.ipAddress = inet_ntoa(clientAddr.sin_addr);
            comp.id = computers.size() + 1;
            comp.isServer = false;
            comp.isAwake = true;

            mtx.lock();
            computers.push_back(comp);
            mtx.unlock();

            strcpy(buffer, DISCOVERY_RESPONSE);

            //int sockfd2 = createSocket(); //Ocorrem diferentes comportamentos se criamos o socketfd2 ou se usamos o que ja existe (sockfd1)
            struct sockaddr_in testeAddr = configureAdress(inet_ntoa(clientAddr.sin_addr), clientAddr.sin_port);
            
            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&testeAddr, sizeof(testeAddr));
        } 

    }

    close(sockfd);
    return 0;
}

int Discovery::client() {
    struct sockaddr_in serverAddr;
    char buffer[MAX_BUFFER_SIZE];

    int sockfd = createSocket();
    setSocketTimeout(sockfd, TIMEOUT_SEC);

    int broadcastPermission = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (void *)&broadcastPermission, sizeof(broadcastPermission)) < 0) {
        cerr << "Error setting socket to broadcast mode" << endl;
        return -1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_DISCOVERY);
    serverAddr.sin_addr.s_addr = inet_addr(BROADCAST_IP);

    string message = DISCOVERY_MESSAGE + string(" MAC- ") + getMacAddress();
    snprintf(buffer, MAX_BUFFER_SIZE, "%s", message.c_str());

    while (true) {
        if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&serverAddr, (socklen_t)sizeof(serverAddr)) == -1) {
            std::cerr << "Erro ao enviar: " << strerror(errno) << std::endl;
        }
        int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&serverAddr, (socklen_t*)sizeof(serverAddr)); // o erro está aqui
        if (bytesReceived < 0) {
            if (isTimeoutError()) {
                cout << "No response from server" << endl;
                continue;
            }
            cout << "deu ruim";
            break;
        }
        else 
            cout << "Fui Descobertooooooooooooo" << endl;
            if (strcmp(buffer, DISCOVERY_RESPONSE) == 0) {
                mtx.lock();
                serverIp = inet_ntoa(serverAddr.sin_addr);
                serverPort = serverAddr.sin_port;
                mtx.unlock();
                break;
            }  
    }
     
    close(sockfd);
    return 0;
}

bool Discovery::isDiscoveryMessage(char* buffer) {
    return strstr(buffer, DISCOVERY_MESSAGE) != NULL;
}