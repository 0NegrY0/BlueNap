#ifndef MONITORING_SUBSERVICE_H
#define MONITORING_SUBSERVICE_H

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

const int MONITORING_PORT = 42000; 
const int MAX_BUFFER_SIZE = 1024;
const int MONITORING_MESSAGE = "Are you Awake?";
const int MONITORING_MESSAGE_RESPONSE = "i am Awake";
const int MONITORING_PORT = 42000;
const int TIMEOUT_SEC = 5;

void handleMonitoringReceiver();
void handleMonitoringSender();

#endif
