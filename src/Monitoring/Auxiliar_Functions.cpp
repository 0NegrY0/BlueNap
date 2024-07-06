#include "src/Monitoring/Headers/Auxiliar_Functions.hpp"

std::string getManagerIp(const std::vector<ComputerInfo>& computers) {
    for (size_t i = 0; i < computers.size(); ++i) {
        if (computers[i].isServer) {
            return computers[i].ipAddress;
        }
    }
    return "";
}

int createSocket() {
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Erro na criação do socket" << std::endl;
        return -1;
    }
    return sockfd;
}

void setSocketTimeout(int sockfd, int sec) {
    struct timeval timeout;
    timeout.tv_sec = sec;
    timeout.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        std::cerr << "Erro ao definir o timeout de recebimento" << std::endl;
    }
}

struct sockaddr_in configureServerAddress(const std::string& ip) {
    struct sockaddr_in Addr;
    memset(&Addr, 0, sizeof(Addr));
    Addr.sin_family = AF_INET;
    Addr.sin_port = htons(MONITORING_PORT);
    if (inet_pton(AF_INET, ip.c_str(), &Addr.sin_addr) <= 0) {
        std::cerr << "Erro ao converter o endereço IP" << std::endl;
        // Não é possível retornar -1, já que a função deve retornar uma estrutura sockaddr_in
        // Vamos retornar uma estrutura inválida para indicar o erro
        Addr.sin_family = AF_UNSPEC;
    }
    return Addr;
}
