#ifndef AUXILIAR_FUNCTIONS_H
#define AUXILIAR_FUNCTIONS_H

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

const int MONITORING_PORT = 42000; 

std::string getManagerIp(const std::vector<ComputerInfo>& computers);

int createSocket();

void setSocketTimeout(int sockfd, int sec);

struct sockaddr_in configureServerAddress(const std::string& ip);

#endif
