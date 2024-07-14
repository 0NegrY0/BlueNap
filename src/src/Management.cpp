#include "../include/Management.hpp"
#include <iostream>
#include <cstring>
#include <cerrno>
#include <arpa/inet.h>
#include <unistd.h>
#include <algorithm>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fstream>
#include <string>

vector<Computer> computers;
mutex mtx;
string serverIp = "";
int serverPort = 0;
string serverHostName = "";
string serverMac = "";
int myPort = 0;
bool shouldExit = false;

string Management::getIPAddress() {
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char addr[INET6_ADDRSTRLEN]; // Alocado espaço suficiente para o endereço IP
    string host; // Usando string para armazenar o endereço IP

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    // Percorre a lista encadeada, mantendo o ponteiro principal para que possamos liberar a lista posteriormente
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) {
            continue;
        }

        family = ifa->ifa_addr->sa_family;

        if (family == AF_INET || family == AF_INET6) {
            s = getnameinfo(ifa->ifa_addr, (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
                            addr, INET6_ADDRSTRLEN, NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                cerr << "getnameinfo() failed: " << gai_strerror(s) << endl;
                exit(EXIT_FAILURE);
            }
            if (strcmp(ifa->ifa_name, "lo") != 0) {
                host = addr; // Copia o endereço IP para o objeto string
                break; // Encontrou uma interface que não é loopback, não precisa iterar mais
            }
        }
    }

    freeifaddrs(ifaddr);
    return host;
}

string Management::getMacAddress() {
    namespace fs = std::filesystem;

    for (const auto& entry : fs::directory_iterator("/sys/class/net/")) {
        std::string interface = entry.path().filename().string();

        // Check if the interface name starts with "enp"
        if (interface.substr(0, 1) == "e") {                    //enp or eth (if there is another interface with e were are done for...)     
            std::ifstream file("/sys/class/net/" + interface + "/address");
            if (file.is_open()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                std::string macAddress = buffer.str();

                // Remove trailing newline
                if (!macAddress.empty() && macAddress[macAddress.length() - 1] == '\n') {
                    macAddress.erase(macAddress.length() - 1);
                }

                if (!macAddress.empty()) {
                    return macAddress;
                }
            }
        }
    }

    throw std::runtime_error("Failed to find a network interface with a MAC address.");
}

string Management::getManagerIp(const vector<Computer>& computers) {
    cout << computers.size() << endl;
    for (size_t i = 0; i < computers.size(); i++) {
        if (computers[i].isServer) {
            return computers[i].ipAddress;
        }
    }
    return "";
}

int Management::createSocket() {
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cerr << "Error in socket creation" << endl;
        return -1;
    }
    return sockfd;
}

void Management::setSocketTimeout(int sockfd, int sec) {
    struct timeval timeout;
    timeout.tv_sec = sec;
    timeout.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        cerr << "Erro ao definir o timeout de recebimento" << endl;
    }
}

struct sockaddr_in Management::configureAdress(const string& ip, int port) {
    struct sockaddr_in Addr;
    memset(&Addr, 0, sizeof(Addr));
    Addr.sin_family = AF_INET;
    Addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &Addr.sin_addr) <= 0) {
        cerr << "Erro ao converter o endereço IP" << endl;
        exit(EXIT_FAILURE);
    }
    return Addr;
}

bool Management::isTimeoutError() {
    return errno == EAGAIN || errno == EWOULDBLOCK;
}

int Management::listenAtPort(int sockfd, int port) {
    if (port != 0) {
        port = htons(port);
    }

    struct sockaddr_in Addr;
    memset(&Addr, 0, sizeof(Addr));
    Addr.sin_family = AF_INET;
    Addr.sin_addr.s_addr = INADDR_ANY;
    Addr.sin_port = port;

    if (bind(sockfd, (struct sockaddr*)&Addr, sizeof(Addr)) < 0) {
        cerr << "Error in bind(): " << strerror(errno) << endl;
        close(sockfd);
        return -1;
    }
    return 0;
}

int Management::askToCloseConnection() {
    int sockfd = createSocket();
            
    struct sockaddr_in serverAddr = configureAdress(serverIp, serverPort);

    char buffer[MAX_BUFFER_SIZE];
    string message = EXIT_MESSAGE;

    snprintf(buffer, MAX_BUFFER_SIZE, "%s", message.c_str());
    sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    return 0;
}

void Management::addComputer(const Computer& computer) {
    mtx.lock();
    computers.push_back(computer);
    mtx.unlock();
}

void Management::removeComputer(const string& ip) {
    mtx.lock();
    computers.erase(std::remove_if(computers.begin(), computers.end(), [ip](const Computer& computer) {
        return computer.ipAddress == ip;
    }), computers.end());
    mtx.unlock();
}

void Management::updateStatus(int id, bool isAwake) {
    mtx.lock();
    for (auto& computer : computers) {
        if (computer.id == id) {
            computer.isAwake = isAwake;
            break;
        }
    }
    mtx.unlock();
}

void Management::wakeOnLan(const string& macAddress, const string& ipAddress) {
    // Cria o pacote mágico
    unsigned char packet[102];
    // Preenche os primeiros 6 bytes com 0xFF
    std::fill(packet, packet + 6, 0xFF);

    // Converte o endereço MAC de string para bytes
    unsigned char mac[6];
    sscanf(macAddress.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);

    // Preenche os próximos 16 blocos de 6 bytes com o endereço MAC
    for (int i = 0; i < 16; ++i) {
        std::copy(mac, mac + 6, packet + 6 + i * 6);
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