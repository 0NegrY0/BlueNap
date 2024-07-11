#include <iostream>
#include <vector>
#include <thread>
#include "../include/Discovery.hpp"
#include "../include/Monitoring.hpp"
#include "../include/Interface.hpp"

using namespace std;

int main() {
    
    int isMaster;

    cout << "Are you the master? (1/0): ";
    cin >> isMaster;

    vector<thread> threads;

    Discovery discovery;
    Monitoring monitoring;
    Interface interface;

    if (isMaster) {
        threads.push_back(thread(discovery.server()));
        threads.push_back(thread(monitoring.server()));
        threads.push_back(thread(interface.server()));
    }
    else {
        threads.push_back(thread(discovery.client()));
        threads.push_back(thread(monitoring.client()));
        threads.push_back(thread(interface.client()));
    }

    return 0;
}

/*
Subservices/Discovey_Subservice.cpp
Subservices/Monitoring_Subservice.cpp
main.cpp
utils.cpp
*/