#include "../include/Management.hpp"
#include <iostream>
#include <cstring>
#include <cerrno>
#include <arpa/inet.h>
#include <unistd.h>
#include <algorithm>

void Management::addComputer(const Computer& computer) {
    mtx.lock();
    computers.push_back(computer);
    mtx.unlock();
}

void Management::removeComputer(int id) {
    mtx.lock();
    for (size_t i = 1; i < computers.size(); ++i) {
        if (computers[i].id == id) {
            computers.erase(computers.begin() + i);
            break;
        }
    }
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
