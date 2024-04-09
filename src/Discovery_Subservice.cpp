#include <iostream>
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
#include <mutex>
#include <vector>
#include <atomic>

#define PORT_DISCOVERY 8888
#define MAX_BUFFER_SIZE 1024
#define DISCOVERY_MESSAGE "sleep service discovery"

std::mutex mtx;
std::atomic<bool> stopSending(false); // Flag para indicar se a função de envio deve parar de enviar mensagens
std::atomic<bool> isDiscovered(false); // Flag para indicar se o computador foi descoberto

// Estrutura para representar um computador
struct Computer {
    std::string macAddress;
    int id;
    bool isServer; // true se for um servidor, false se for um cliente
};

std::vector<Computer> computers; // Vetor para armazenar informações dos computadores

// Função para lidar com a descoberta de estações participantes
void handleDiscoveryReceiver() {
    int sockfd;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    char buffer[MAX_BUFFER_SIZE];

    // Cria socket UDP para descoberta
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Error in socket creation" << std::endl;
        return;
    }

    // Configura o endereço do servidor
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT_DISCOVERY);

    // Liga o socket ao endereço do servidor
    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error in bind()" << std::endl;
        close(sockfd);
        return;
    }

    std::cout << "Discovery service listening on port " << PORT_DISCOVERY << std::endl;

    while (true) {
        memset(buffer, 0, sizeof(buffer));

        // Recebe pacote de descoberta
        int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&clientAddr, &clientLen);
        if (bytesReceived < 0) {
            std::cerr << "Error in recvfrom()" << std::endl;
            continue;
        }

        // Verifica se o pacote recebido é uma mensagem de descoberta
        if (strcmp(buffer, DISCOVERY_MESSAGE) == 0) {
            std::cout << "Received discovery message from: " << inet_ntoa(clientAddr.sin_addr) << std::endl;

            // Cria uma nova estrutura Computer para armazenar informações sobre o computador
            Computer comp;
            comp.macAddress = "Sample MAC"; // Substituir por lógica real para obter o endereço MAC
            comp.id = computers.size() + 1;
            comp.isServer = false; // Assumindo que todos os computadores descobertos são clientes

            // Adquire exclusão mútua ao adicionar um novo computador ao vetor
            mtx.lock();
            computers.push_back(comp);
            mtx.unlock();

            // Informa que o computador foi descoberto
            isDiscovered = true;
        }
    }

    // Fecha o socket
    close(sockfd);
}

// Função para enviar mensagens de descoberta
void handleDiscoverySender() {
    int sockfd;
    struct sockaddr_in serverAddr;
    char buffer[MAX_BUFFER_SIZE];

    // Cria socket UDP para descoberta
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Error in socket creation" << std::endl;
        return;
    }

    // Configura o endereço do servidor
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_DISCOVERY);
    serverAddr.sin_addr.s_addr = INADDR_BROADCAST;

    while (true) {
        if (!isDiscovered) { // Verifica se o computador ainda não foi descoberto
            // Envia mensagem de descoberta
            strcpy(buffer, DISCOVERY_MESSAGE);
            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
            sleep(1); // Aguarda um segundo entre os envios
        } else {
            break; // Se o computador foi descoberto, encerra o envio
        }
    }

    // Fecha o socket
    close(sockfd);
}

int main() {
    std::vector<std::thread> threads;

    // Adiciona threads para lidar com a descoberta de estações participantes
    threads.push_back(std::thread(handleDiscoveryReceiver));

    // Adiciona threads para enviar mensagens de descoberta
    threads.push_back(std::thread(handleDiscoverySender));

    // Aguarda a finalização de todas as threads
    for (auto& th : threads) {
        th.join();
    }

    return 0;
}
