#include <iostream>
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
#include <mutex>
#include <vector>
#include <atomic>

#define MAX_BUFFER_SIZE 1024
#define MONITORING_MESSAGE "Are you Awake?"
#define MONITORING_MESSAGE_RESPONSE "i am Awake"
#define MONITORING_PORT 42000
#define TIMEOUT_SEC 5
using namespace std;

mutex mtx;
atomic<bool> stopSending(false); // Flag para indicar se a função de envio deve parar de enviar mensagens
atomic<bool> isDiscovered(false); // Flag para indicar se o computador foi descoberto

// Estrutura para representar um computador
struct Computer {
    string macAddress;
    int id;
    bool isServer; // true se for um servidor, false se for um cliente
};

vector<Computer> computers; // Vetor para armazenar informações dos computadores
bool messageSent;

void setSocketTimeout(int sockfd, int sec) {
    struct timeval timeout;
    timeout.tv_sec = sec;
    timeout.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        std::cerr << "Erro ao definir o timeout de recebimento" << std::endl;
        return 1;
    }
}

string getManagerIp() {
    for (int i = 0; i < computers.size(); i++) {
        if (computers[i].isServer) {
            return computers[i].ip;
        }
    }
    return "";
}

// Função para lidar com a descoberta de estações participantes
void handleMonitoringReceiver() {
    int sockfd;
    struct sockaddr_in managerAddr;
    char buffer[MAX_BUFFER_SIZE];

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cerr << "Error in socket creation" << endl;
        return;
    }

    string managerIp = getManagerIp();
    if (managerIp == "") {
        cerr << "Erro ao obter o IP do gerenciador" << endl;
        return;
    }

    memset(&managerAddr, 0, sizeof(managerAddr));
    managerAddr.sin_family = AF_INET;
    managerAddr.sin_port = htons(MONITORING_PORT);
    if (inet_pton(AF_INET, managerIp, &managerAddr.sin_addr) <= 0) {
        std::cerr << "Erro ao converter o endereço IP" << std::endl;
        return 1;
    }

    while (true) {
        int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&managerAddr, sizeof(managerAddr));
        if (bytesReceived < 0) {
            std::cerr << "Error in recvfrom()" << std::endl;
            continue;
        }
        else {
            if (strcmp(buffer, MONITORING_MESSAGE) == 0) {
                strcpy(buffer, MONITORING_MESSAGE_RESPONSE);
                sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&managerAddr, sizeof(managerAddr));
            }
        }
    }
}

// Função para enviar mensagens de monitoramento
void handleMonitoringSender() {
    int sockfd;
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    char buffer[MAX_BUFFER_SIZE];

    // Cria socket UDP para descoberta
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Error in socket creation" << std::endl;
        return;
    }

    setSocketTimeout(sockfd, TIMEOUT_SEC);

    while (true) {
        for (int i = 0; i < computers.size(); i++) {
            memset(&clientAddr, 0, sizeof(clientAddr));
            clientAddr.sin_family = AF_INET;
            clientAddr.sin_port = htons(MONITORING_PORT);
            if (inet_pton(AF_INET, computers[i].ip, &clientAddr.sin_addr) <= 0) {
                std::cerr << "Erro ao converter o endereço IP" << std::endl;
                return 1;
            }

            strcpy(buffer, MONITORING_MESSAGE);
            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));

            int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
            if (bytesReceived < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) { //PC ta dormindo
                    computers[i].isAwake = false;
                }
                else {
                    std::cerr << "Error in recvfrom()" << std::endl;
                    continue;
                }
            }
            else {
                if (strcmp(buffer, MONITORING_MESSAGE_RESPONSE) == 0) {
                    computers[i].isAwake = true;
                }
            }
        }
    }

    // Fecha o socket
    close(sockfd);
}

int main() {
    vector<thread> threads;

    // Adiciona threads para lidar com a descoberta de estações participantes
    threads.push_back(thread(handleDiscoveryReceiver));

    // Adiciona threads para enviar mensagens de descoberta
    threads.push_back(thread(handleDiscoverySender));

    // Aguarda a finalização de todas as threads
    for (auto& th : threads) {
        th.join();
    }

    return 0;
}