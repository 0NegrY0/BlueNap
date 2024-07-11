#include <iostream>
#include <vector>
#include <thread>
#include "../include/Discovery.hpp"
#include "../include/Monitoring.hpp"
#include "../include/Management.hpp"
#include "../include/Interface.hpp"

using namespace std;

int main() {
    
    int isMaster;

    cout << "Are you the master? (1/0): ";
    cin >> isMaster;

    vector<thread> threads;

    Discovery discovery;
    Monitoring monitoring;
    Management management;
    Interface interface;

    if (isMaster) {
        Computer comp;
        comp.macAddress = discovery.getMacAddress();
        comp.ipAddress = discovery.getMacAddress();
        comp.id = 1;
        comp.isServer = true;
        threads.push_back(thread(&Discovery::server, &discovery));
        threads.push_back(thread(&Monitoring::server, &monitoring));
        threads.push_back(thread(&Interface::server, &interface));
    }
    else {
        threads.push_back(thread(&Discovery::client, &discovery));
        threads.push_back(thread(&Monitoring::client, &monitoring));
        threads.push_back(thread(&Interface::client, &interface));
    }

    return 0;
}