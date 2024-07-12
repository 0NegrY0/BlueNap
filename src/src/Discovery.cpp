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
        cerr << "Error in bind(): " << strerror(errno) << endl;
        close(sockfd);
        return -1;
    }

    cout << "Discovery service listening on port " << PORT_DISCOVERY << endl;

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&clientAddr, &clientLen);
        if (bytesReceived < 0) {
            cerr << "Error in recvfrom(): " << strerror(errno) << endl;
            continue;
        }

        if (isDiscoveryMessage(buffer)) {
            cout << "Received discovery message from: " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << endl;
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

            cout << "Sending discovery response to: " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << endl;

            // Send response back to the client's port
            if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&clientAddr, clientLen) < 0) {
                cerr << "Error in sendto(): " << strerror(errno) << endl;
            } else {
                cout << "Response sent successfully to: " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << endl;
            }
        }
    }

    close(sockfd);
    return 0;
}


int Discovery::client() {
    struct sockaddr_in serverAddr, localAddr, responseAddr;
    char buffer[MAX_BUFFER_SIZE];

    int sockfd = createSocket();
    setSocketTimeout(sockfd, TIMEOUT_SEC);

    int broadcastPermission = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (void *)&broadcastPermission, sizeof(broadcastPermission)) < 0) {
        cerr << "Error setting socket to broadcast mode: " << strerror(errno) << endl;
        return -1;
    }

    // Bind to a random local port to receive the response
    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = INADDR_ANY;
    localAddr.sin_port = 0;  // 0 allows the OS to choose a random available port

    if (bind(sockfd, (struct sockaddr*)&localAddr, sizeof(localAddr)) < 0) {
        cerr << "Error in bind(): " << strerror(errno) << endl;
        close(sockfd);
        return -1;
    }

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
        } else {
            cout << "Discovery message sent to broadcast address" << endl;
        }

        int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&responseAddr, &responseAddrLen);
        if (bytesReceived < 0) {
            if (isTimeoutError()) {
                cout << "No response from server" << endl;
                continue;
            }
            cerr << "Error in recvfrom(): " << strerror(errno) << endl;
            break;
        } else {
            cout << "Received response from server: " << inet_ntoa(responseAddr.sin_addr) << ":" << ntohs(responseAddr.sin_port) << endl;
            buffer[bytesReceived] = '\0';  // Null-terminate the received data
            if (strcmp(buffer, DISCOVERY_RESPONSE) == 0) {
                mtx.lock();
                serverIp = inet_ntoa(responseAddr.sin_addr);
                serverPort = ntohs(responseAddr.sin_port);
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
