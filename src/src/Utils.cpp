#include "../include/Utils.hpp"
#include <iostream>
#include <cstring>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <unistd.h>

using namespace std;

vector<Computer> computers;
mutex mtx;
string serverIp = "";
int serverPort = 0;
string serverHostName = "";
string serverMac = "";
int myPort = 0;
bool hasUpdate = false;

string Utils::getIPAddress() {
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

string Utils::getMacAddress() {
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

string Utils::getManagerIp(const vector<Computer>& computers) {
    cout << computers.size() << endl;
    for (size_t i = 0; i < computers.size(); i++) {
        if (computers[i].isServer) {
            return computers[i].ipAddress;
        }
    }
    return "";
}

int Utils::createSocket() {
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cerr << "Error in socket creation" << endl;
        return -1;
    }
    return sockfd;
}

void Utils::setSocketTimeout(int sockfd, int sec) {
    struct timeval timeout;
    timeout.tv_sec = sec;
    timeout.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        cerr << "Erro ao definir o timeout de recebimento" << endl;
    }
}

struct sockaddr_in Utils::configureAdress(const string& ip, int port) {
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

bool Utils::isTimeoutError() {
    return errno == EAGAIN || errno == EWOULDBLOCK;
}

int Utils::listenAtPort(int sockfd, int port) {
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
