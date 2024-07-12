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
        cout << "Received discovery message from: " << inet_ntoa(clientAddr.sin_addr) << endl;

        if (isDiscoveryMessage(buffer)) {
            cout << "Received discovery message from: " << inet_ntoa(clientAddr.sin_addr) << endl;

            string message(buffer);
            string ip, mac;
            size_t ipPos = message.find("IP:");
            size_t macPos = message.find(", MAC:");
            if (ipPos != string::npos && macPos != string::npos) {
                ip = message.substr(ipPos + 4, macPos - (ipPos + 4));
                mac = message.substr(macPos + 7);
            } else {
                cerr << "Invalid discovery message format" << endl;
                continue;
            }

            Computer comp;
            comp.macAddress = mac;
            comp.ipAddress = ip;
            comp.id = computers.size() + 1;
            comp.isServer = false;

            mtx.lock();
            computers.push_back(comp);
            mtx.unlock();

            strcpy(buffer, DISCOVERY_RESPONSE);
            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
        } 

    }

    close(sockfd);
    return 0;
}

int Discovery::client() {
    int sockfd;
    struct sockaddr_in serverAddr;
    char buffer[MAX_BUFFER_SIZE];

    // Create UDP socket for discovery
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cerr << "Error in socket creation" << endl;
        return -1;
    }

    setSocketTimeout(sockfd, TIMEOUT_SEC);

    // Configure server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_DISCOVERY);
    serverAddr.sin_addr.s_addr = (in_addr_t) INADDR_BROADCAST;

    sprintf(buffer, "%s IP:%s, MAC:%s", DISCOVERY_MESSAGE, getIPAddress(), getMacAddress());

    while (true) {
        sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&serverAddr, (socklen_t)sizeof(serverAddr));
        int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&serverAddr, (socklen_t*)sizeof(serverAddr));
        if (bytesReceived < 0) {
            if (isTimeoutError()) {
                cout << "No response from server" << endl;
                continue;
            }
            break;
        }
        else if (strcmp(buffer, DISCOVERY_RESPONSE) == 0) {
            cout << "Received discovery message from: " << inet_ntoa(serverAddr.sin_addr) << endl;
            break;
        }  
        sleep(10);
    }
     
    close(sockfd);
    return 0;
}

bool Discovery::isDiscoveryMessage(char* buffer) {
    return strstr(buffer, DISCOVERY_MESSAGE) != NULL;
}

bool Discovery::isTimeoutError() {
    return errno == EAGAIN || errno == EWOULDBLOCK;
}