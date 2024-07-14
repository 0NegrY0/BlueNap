#include "../include/Discovery.hpp"
#include "../include/Management.hpp"
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

        if (isExitMessage(buffer)) {
            Management management;
            for (size_t i = 0; i < computers.size(); i++){
                string ipToCompare(inet_ntoa(clientAddr.sin_addr));
                if (computers[i].ipAddress == ipToCompare) {
                    computers.erase(computers.begin() + i);
                    break;
                }
            }
        }

        if (isDiscoveryMessage(buffer)) {
            string message(buffer);
            size_t macPos = message.find("MAC- ");
            size_t namePos = message.find("Name- ");

            if (macPos == string::npos || namePos == string::npos) {
                cerr << "Invalid discovery message format" << endl;
                continue;
            }

            string name = message.substr(namePos + strlen("Name- "));

            string ip = inet_ntoa(clientAddr.sin_addr);
            macPos = macPos + strlen("MAC- ");
            string mac = message.substr(macPos, namePos - macPos);
            
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
                comp.hostName = name;
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

    char hostname[1024];
    gethostname(hostname, 1024);

    string message = DISCOVERY_MESSAGE + string(" MAC- ") + getMacAddress() + string("Name- ") + hostname; //ADICIONAR HOST NAME
    snprintf(buffer, MAX_BUFFER_SIZE, "%s", message.c_str());

    socklen_t addrLen = sizeof(serverAddr);
    socklen_t responseAddrLen = sizeof(responseAddr);

    while (!shouldExit) {
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
                size_t hostNamePos = message2.find("Host Name:");
                size_t macPos = message2.find("Host Mac:");

                if (portPos == string::npos || hostNamePos == string::npos || macPos == string::npos) {
                    cerr << "Invalid discovery message format" << endl;
                    continue;
                }

                int port = stoi(message2.substr(portPos + strlen("Port:"), 5));
                hostNamePos = hostNamePos + strlen("Host Name:");

                string hostName = message2.substr(hostNamePos, macPos - hostNamePos);

                macPos = macPos + strlen("Host Mac:");

                string hostMac = message2.substr(macPos, portPos - macPos);

                mtx.lock();
                serverIp = inet_ntoa(responseAddr.sin_addr);
                serverPort = ntohs(responseAddr.sin_port);
                serverHostName = hostName;
                serverMac = hostMac;
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

bool Discovery::isExitMessage(char* buffer) {
    return strstr(buffer, EXIT_MESSAGE) != NULL;
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

    char hostname[1024];
    gethostname(hostname, 1024);
    string hostnameStr(hostname);

    string response = DISCOVERY_RESPONSE + string("Host Name:") + hostnameStr + "Host Mac:" + getMacAddress() + string("Port:") + to_string(clientPort);
    snprintf(discoveryMessage, MAX_BUFFER_SIZE, "%s", response.c_str());

    return discoveryMessage;
}