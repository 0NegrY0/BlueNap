#include "Headers/Discovery_Subservice.hpp"
#include "Headers/Auxiliar_Functions.hpp"
#include <iostream>
#include <cstring>
#include <cerrno>
#include <arpa/inet.h>
#include <unistd.h>

std::mutex mtx;
std::atomic<bool> stopSending(false); // Flag to indicate if the sending function should stop sending messages
std::atomic<bool> isDiscovered(false); // Flag to indicate if the computer has been discovered
std::vector<Computer> computers; // Vector to store information of the computers

// Function to handle discovery messages reception
void handleDiscoveryReceiver() {
    int sockfd;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    char buffer[MAX_BUFFER_SIZE];

    // Create UDP socket for discovery
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Error in socket creation" << std::endl;
        return;
    }

    // Configure server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT_DISCOVERY);

    // Bind socket to server address
    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error in bind()" << std::endl;
        close(sockfd);
        return;
    }

    std::cout << "Discovery service listening on port " << PORT_DISCOVERY << std::endl;

    while (true) {
        memset(buffer, 0, sizeof(buffer));

        // Receive discovery packet
        int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&clientAddr, &clientLen);
        if (bytesReceived < 0) {
            std::cerr << "Error in recvfrom()" << std::endl;
            continue;
        }

        // Check if received packet is a discovery message
        if (strcmp(buffer, DISCOVERY_MESSAGE) == 0) {
            std::cout << "Received discovery message from: " << inet_ntoa(clientAddr.sin_addr) << std::endl;

            // Extract IP and MAC address from the message
            std::string message(buffer);
            std::string ip, mac;
            size_t ipPos = message.find("IP:");
            size_t macPos = message.find(", MAC:");
            if (ipPos != std::string::npos && macPos != std::string::npos) {
                ip = message.substr(ipPos + 4, macPos - (ipPos + 4));
                mac = message.substr(macPos + 7);
            } else {
                std::cerr << "Invalid discovery message format" << std::endl;
                continue;
            }

            // Create a new Computer structure to store information about the computer
            Computer comp;
            comp.macAddress = mac;
            comp.ipAddress = ip;
            comp.id = computers.size() + 1;
            comp.isServer = false; // Assuming all discovered computers are clients

            // Acquire mutual exclusion when adding a new computer to the vector
            mtx.lock();
            computers.push_back(comp);
            mtx.unlock();

            // Inform that the computer has been discovered
            isDiscovered = true;
        }
    }

    // Close the socket
    close(sockfd);
}

// Function to send discovery messages
void handleDiscoverySender() {
    int sockfd;
    struct sockaddr_in serverAddr;
    char buffer[MAX_BUFFER_SIZE];

    // Create UDP socket for discovery
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Error in socket creation" << std::endl;
        return;
    }

    // Configure server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_DISCOVERY);
    serverAddr.sin_addr.s_addr = INADDR_BROADCAST;

    while (true) {
        if (!isDiscovered) { // Check if the computer has not been discovered yet
            // Prepare discovery message with placeholders for IP and MAC address
            sprintf(buffer, "%s", DISCOVERY_MESSAGE);
            
            // Send discovery message
            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
            sleep(1); // Wait one second between sends
        } else {
            break; // If the computer has been discovered, stop sending
        }
    }

    // Close the socket
    close(sockfd);
}

int main() {
    std::vector<std::thread> threads;

    // Add threads to handle discovery messages reception
    threads.push_back(std::thread(handleDiscoveryReceiver));

    // Add threads to send discovery messages
    threads.push_back(std::thread(handleDiscoverySender));

    // Wait for all threads to finish
    for (auto& th : threads) {
        th.join();
    }

    return 0;
}
