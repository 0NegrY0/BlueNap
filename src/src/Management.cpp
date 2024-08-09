#include "../include/Management.hpp"
#include "../include/Utils.hpp"
#include <iostream>
#include <cstring>
#include <cerrno>
#include <arpa/inet.h>
#include <unistd.h>
#include <algorithm>

void Management::addComputer(const Computer& computer) {
    mtx.lock();
    computers.push_back(computer);
    internalClock++;
    mtx.unlock();
}

Computer Management::createComputer(string clientIp, string clientMac) {
    Computer comp;
    comp.macAddress = clientMac;
    comp.ipAddress = clientIp;
    comp.id = nextID;
    nextID++;
    comp.isServer = false;
    comp.isAwake = true;
    comp.port = PORT_DISCOVERY + comp.id;

    return comp;
}

void Management::removeComputer(int id) {
    mtx.lock();
    for (size_t i = 1; i < computers.size(); ++i) {
        cout << "Entrou no for" << endl;
        sleep(5);
        if (computers[i].id == id) {
            cout << "vou apagar" << computers[i].ipAddress << endl;
            computers.erase(computers.begin() + i);
            break;
        }
    }
    internalClock++;
    mtx.unlock();
}

void Management::updateStatus(int id, bool isAwake) {
    mtx.lock();
    for (auto& comp : computers) {
        if (comp.id == id) {
            if (comp.isAwake != isAwake) {
                comp.isAwake = isAwake;
                internalClock++;
            }
            break;
        }
    }
    mtx.unlock();
}

int Management::getPort(int computerId) {
    int port;
    mtx.lock();
    for (auto& computer : computers) {
        if (computer.id == computerId) {
            port = computer.port;
        }
    }
    mtx.unlock();
    return port;
}

void Management::wakeOnLan(const string& macAddress, const string& ipAddress) {
    // Cria o pacote mágico
    unsigned char packet[102];
    // Preenche os primeiros 6 bytes com 0xFF
    fill(packet, packet + 6, 0xFF);

    // Converte o endereço MAC de string para bytes
    unsigned char mac[6];
    sscanf(macAddress.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);

    // Preenche os próximos 16 blocos de 6 bytes com o endereço MAC
    for (int i = 0; i < 16; ++i) {
        copy(mac, mac + 6, packet + 6 + i * 6);
    }

    // Configura o endereço de destino
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        cerr << "Erro ao criar socket" << endl;
        return;
    }
    
    int broadcastEnable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0) {
        cerr << "Erro ao habilitar broadcast" << endl;
        close(sockfd);
        return;
    }

    struct sockaddr_in destAddr;
    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(9); // Porta padrão para WoL
    inet_pton(AF_INET, ipAddress.c_str(), &destAddr.sin_addr);

    // Envia o pacote mágico
    if (sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr*)&destAddr, sizeof(destAddr)) < 0) {
        cerr << "Erro ao enviar pacote WoL" << endl;
    } else {
        cout << "Pacote WoL enviado para " << macAddress << " (" << ipAddress << ")" << endl;
    }

    // Fecha o socket
    close(sockfd);
}

void Management::startElection(int initiator) {
    int myId = myPort - PORT_DISCOVERY;
    cout << "Process " << myId << " started an election." << endl;
    bool amILeader = true;
    char buffer[MAX_BUFFER_SIZE];

    for (auto& comp : computers) {
        if (comp.isAwake && comp.id < myId) {
            int sockfd = createSocket();
            setSocketTimeout(sockfd, 1);

            struct sockaddr_in clientAddr = configureAdress(comp.ipAddress, comp.port);
            socklen_t clientLen = sizeof(clientAddr);

            string message = ELECTION_MESSAGE + to_string(myId);
            char* electionMessage = new char[MAX_BUFFER_SIZE];
            snprintf(electionMessage, MAX_BUFFER_SIZE, "%s", message.c_str());

            sendto(sockfd, electionMessage, strlen(electionMessage), 0, (struct sockaddr*)&clientAddr, clientLen);

            clientAddr = configureAdress(comp.ipAddress, comp.port);
            int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&clientAddr, &clientLen);
            if (bytesReceived > 0) {
                if (isMessage(buffer, ELECTION_RESPONSE)) {
                    amILeader = false;
                    break;
                }
            }
        }
    }
    if (amILeader) {
        announceElectionResult();
    }
}

void Management::announceElectionResult() {
    char* electionResult = new char[MAX_BUFFER_SIZE];

    char hostname[1024];
    gethostname(hostname, 1024);
    string hostnameStr(hostname);

    string response = ELECTION_RESULT + string("Host Name:") + hostnameStr + "Host Mac:" + getMacAddress();
    snprintf(electionResult, MAX_BUFFER_SIZE, "%s", response.c_str());

    for (auto& comp : computers) {
        if (comp.isAwake) {
            int sockfd = createSocket();

            struct sockaddr_in clientAddr = configureAdress(comp.ipAddress, comp.port);
            socklen_t clientLen = sizeof(clientAddr);

            sendto(sockfd, electionResult, strlen(electionResult), 0, (struct sockaddr*)&clientAddr, clientLen);
        }
    }
    isMaster = 1;
}

Computer Management::deserialize(const char* data, size_t& bytesRead) {
    Computer comp;
    size_t offset = 0;

    // Deserialize the string macAddress
    const char* macAddressStart = data + offset;
    const char* macAddressEnd = find(macAddressStart, macAddressStart + 256, '\0');
    comp.macAddress.assign(macAddressStart, macAddressEnd);
    offset += (macAddressEnd - macAddressStart) + 1; // +1 for the null character

    // Deserialize the string ipAddress
    const char* ipAddressStart = data + offset;
    const char* ipAddressEnd = find(ipAddressStart, ipAddressStart + 256, '\0');
    comp.ipAddress.assign(ipAddressStart, ipAddressEnd);
    offset += (ipAddressEnd - ipAddressStart) + 1; // +1 for the null character

    // Deserialize the string hostName
    const char* hostNameStart = data + offset;
    const char* hostNameEnd = find(hostNameStart, hostNameStart + 256, '\0');
    comp.hostName.assign(hostNameStart, hostNameEnd);
    offset += (hostNameEnd - hostNameStart) + 1; // +1 for the null character

    // Deserialize the integer id
    memcpy(&comp.id, data + offset, sizeof(int));
    offset += sizeof(int);

    // Deserialize the bool isServer
    memcpy(&comp.isServer, data + offset, sizeof(bool));
    offset += sizeof(bool);

    // Deserialize the bool isAwake
    memcpy(&comp.isAwake, data + offset, sizeof(bool));
    offset += sizeof(bool);

    // Deserialize the integer port
    memcpy(&comp.port, data + offset, sizeof(int));
    offset += sizeof(int);

    // Update the total bytes read
    bytesRead = offset;

    return comp;
}

vector<char> Management::setMonitoringMessage() {
    string message = MONITORING_MESSAGE + to_string(internalClock);

    int messageSize = message.length();

    int vecSize = computers.size();
    int totalSize = messageSize + 1 + sizeof(int); // +1 for null terminator

    for (const auto& comp : computers) {
        totalSize += comp.macAddress.size() + 1;
        totalSize += comp.ipAddress.size() + 1;
        totalSize += comp.hostName.size() + 1;
        totalSize += sizeof(int);
        totalSize += sizeof(bool);
        totalSize += sizeof(bool);
        totalSize += sizeof(int);
    }

    vector<char> buffer(totalSize);

    char* bufferPos = buffer.data();
    memcpy(bufferPos, message.c_str(), messageSize + 1); // +1 for null terminator
    bufferPos += messageSize + 1;

    memcpy(bufferPos, &vecSize, sizeof(int));
    bufferPos += sizeof(int);

    for (const auto& comp : computers) {
        memcpy(bufferPos, comp.macAddress.c_str(), comp.macAddress.size() + 1);
        bufferPos += comp.macAddress.size() + 1;

        memcpy(bufferPos, comp.ipAddress.c_str(), comp.ipAddress.size() + 1);
        bufferPos += comp.ipAddress.size() + 1;

        memcpy(bufferPos, comp.hostName.c_str(), comp.hostName.size() + 1);
        bufferPos += comp.hostName.size() + 1;

        memcpy(bufferPos, &comp.id, sizeof(int));
        bufferPos += sizeof(int);

        memcpy(bufferPos, &comp.isServer, sizeof(bool));
        bufferPos += sizeof(bool);

        memcpy(bufferPos, &comp.isAwake, sizeof(bool));
        bufferPos += sizeof(bool);

        memcpy(bufferPos, &comp.port, sizeof(int));
        bufferPos += sizeof(int);
    }

    return buffer;
}

void Management::receiveComputers(vector<char>& buffer) {
    const char* currentPos = buffer.data();
    
    string message(currentPos);
    size_t pos = message.find(MONITORING_MESSAGE);
    if (pos == string::npos) {
        cerr << "Monitoring message not found" << endl;
        return;
    }

    int clockReceived = stoi(message.substr(pos + strlen(MONITORING_MESSAGE)));
    currentPos += message.size() + 1;
    if (clockReceived > internalClock) {
        int vecSize;
        memcpy(&vecSize, currentPos, sizeof(int));
        currentPos += sizeof(int); 

        computers.resize(vecSize);
        for (int i = 0; i < vecSize; ++i) {
            size_t bytesRead = 0;
            computers[i] = deserialize(currentPos, bytesRead);
            currentPos += bytesRead;
        }
        internalClock = clockReceived;
        if (isMaster == 1) {
            isMaster = 0;
        }
    }
}
