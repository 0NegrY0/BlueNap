#include "../include/Discovery.hpp"
#include <iostream>
#include <cstring>
#include <cerrno>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

int Discovery::server() {
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    char buffer[MAX_BUFFER_SIZE];
    int sockfd = createSocket();

    listenAtPort(sockfd, PORT_DISCOVERY);

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&clientAddr, &clientLen);
        if (bytesReceived < 0) {
            cerr << "Error in recvfrom(): " << strerror(errno) << endl;
            continue;
        }

        if (isDiscoveryMessage(buffer)) {
            string message(buffer);
            size_t macPos = message.find("MAC- ");

            if (macPos == string::npos) {
                cerr << "Invalid discovery message format" << endl;
                continue;
            }

            string ip = inet_ntoa(clientAddr.sin_addr);
            string mac = message.substr(macPos + 5, 17);

            int port;
            
            int computerId = isAlreadyDiscovered(ip);
            if (computerId != -1) {
                mtx.lock();
                computers[computerId - 1].isAwake = true;
                mtx.unlock();
                port = computers[computerId - 1].port;
            }

            else {
                Computer comp = createComputer(ip, mac);
                mtx.lock();
                computers.push_back(comp);
                mtx.unlock();
                port = comp.port;
            }

            strcpy(buffer, setDiscoveryResponse(port));

            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&clientAddr, clientLen);
            
        }
    }

    close(sockfd);
    return 0;
}

int Discovery::client() {
    struct sockaddr_in serverAddr, responseAddr;
    char buffer[MAX_BUFFER_SIZE];

    int sockfd = createSocket();
    setSocketTimeout(sockfd, TIMEOUT_SEC);

    int broadcastPermission = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (void *)&broadcastPermission, sizeof(broadcastPermission)) < 0) {
        cerr << "Error setting socket to broadcast mode: " << strerror(errno) << endl;
        return -1;
    }

    listenAtPort(sockfd, 0);

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_DISCOVERY);
    serverAddr.sin_addr.s_addr = inet_addr(BROADCAST_IP);

    string message = DISCOVERY_MESSAGE + string(" MAC- ") + getMacAddress();
    snprintf(buffer, MAX_BUFFER_SIZE, "%s", message.c_str());

    socklen_t addrLen = sizeof(serverAddr);
    socklen_t responseAddrLen = sizeof(responseAddr);

    while (true) {
        if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&serverAddr, addrLen) == -1) {
            cerr << "Error sending discovery message: " << strerror(errno) << endl;
        } 

        int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&responseAddr, &responseAddrLen);
        if (bytesReceived < 0) {
            if (isTimeoutError()) {
                //cout << "No response from server" << endl;
                continue;
            }
            cerr << "Error in recvfrom(): " << strerror(errno) << endl;
            break;
        } 
        else {
            if (isDiscoveryResponse(buffer)) {
                string message2(buffer);
                size_t portPos = message2.find("Port:");

                if (portPos == string::npos) {
                    cerr << "Invalid discovery message format" << endl;
                    continue;
                }

                int port = stoi(message2.substr(portPos + strlen("Port:"), 5));
                mtx.lock();
                serverIp = inet_ntoa(responseAddr.sin_addr);
                serverPort = ntohs(responseAddr.sin_port);
                myPort = port;
                mtx.unlock();

                break;
            }
        }
    }

    close(sockfd);
    return 0;
}

bool Discovery::isDiscoveryMessage(char* buffer) {
    return strstr(buffer, DISCOVERY_MESSAGE) != NULL;
}

bool Discovery::isDiscoveryResponse(char* buffer) {
    return strstr(buffer, DISCOVERY_RESPONSE) != NULL;
}

Computer Discovery::createComputer(string clientIp, string clientMac) {
    Computer comp;
    comp.macAddress = clientMac;
    comp.ipAddress = clientIp;
    comp.id = computers.size() + 1;
    comp.isServer = false;
    comp.isAwake = true;
    comp.port = PORT_DISCOVERY + comp.id;

    return comp;
}

int Discovery::isAlreadyDiscovered(string clientIp) {
    for (Computer c : computers) {
        if (c.ipAddress == clientIp) {
            return c.id;
        }
    }
    return -1;
}

char* Discovery::setDiscoveryResponse(int clientPort) {
    char* discoveryMessage = new char[MAX_BUFFER_SIZE];

    string response = DISCOVERY_RESPONSE + string("Port:") + to_string(clientPort);
    snprintf(discoveryMessage, MAX_BUFFER_SIZE, "%s", response.c_str());

    return discoveryMessage;
}