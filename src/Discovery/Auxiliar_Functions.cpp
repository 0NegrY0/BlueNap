#include "Headers/Auxiliar_Functions.hpp"

namespace fs = std::filesystem;


std::string getIPAddress() {
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char addr[INET6_ADDRSTRLEN]; // Alocado espaço suficiente para o endereço IP
    std::string host; // Usando std::string para armazenar o endereço IP

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
                std::cerr << "getnameinfo() failed: " << gai_strerror(s) << std::endl;
                exit(EXIT_FAILURE);
            }
            if (strcmp(ifa->ifa_name, "lo") != 0) {
                host = addr; // Copia o endereço IP para o objeto std::string
                break; // Encontrou uma interface que não é loopback, não precisa iterar mais
            }
        }
    }

    freeifaddrs(ifaddr);
    return host;
}


std::string getMacAddress() {
    for (const auto& entry : fs::directory_iterator("/sys/class/net/")) {
        std::string interface = entry.path().filename();

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

    throw std::runtime_error("Failed to find a network interface with a MAC address.");
}
