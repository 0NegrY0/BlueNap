#include <iostream>
#include <unistd.h>
#include <vector>
#include <thread>
#include "../include/Discovery.hpp"
#include "../include/Monitoring.hpp"
#include "../include/Management.hpp"
#include "../include/Interface.hpp"
#include <future>

using namespace std;

void joinThreads(vector<thread>& threads) {
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}

void client(vector<thread>& threads, shared_ptr<Discovery> discovery, shared_ptr<Monitoring> monitoring, shared_ptr<Interface> interface) {
    threads.push_back(thread(&Discovery::client, discovery.get()));
    threads.push_back(thread(&Monitoring::client, monitoring.get()));
    threads.push_back(thread(&Interface::client, interface.get()));

    joinThreads(threads);

    threads.clear();

    if (isMaster) {
        server(threads, discovery, monitoring, interface);
    }
}

void server(vector<thread>& threads, shared_ptr<Discovery> discovery, shared_ptr<Monitoring> monitoring, shared_ptr<Interface> interface) {
    threads.push_back(thread(&Discovery::server, discovery.get()));
    threads.push_back(thread(&Monitoring::server, monitoring.get()));
    threads.push_back(thread(&Interface::server, interface.get()));

    joinThreads(threads);

    threads.clear();
    
    if (!isMaster) { // precisa atualizar o valor pro server antigo
        client(threads, discovery, monitoring, interface);
    }
}

int main(int agrc, char* agrv[]) {

    cout << "Are you the master? (1/0): ";
    cin >> isMaster;

    vector<thread> threads;
      
    // OLD IMPLEMENTATION - WORKING
    // Usar unique_ptr para gerenciar a memória automaticamente
    // unique_ptr<Discovery> discovery = make_unique<Discovery>();
    // unique_ptr<Monitoring> monitoring = make_unique<Monitoring>();
    // unique_ptr<Management> management = make_unique<Management>();
    // unique_ptr<Interface> interface = make_unique<Interface>();

     // Use shared_ptr to manage memory automatically
    shared_ptr<Discovery> discovery = make_shared<Discovery>();
    shared_ptr<Monitoring> monitoring = make_shared<Monitoring>();
    shared_ptr<Management> management = make_shared<Management>();
    shared_ptr<Interface> interface = make_shared<Interface>();
    
    while(true) {

        try {
            Management management;
            if (isMaster) {
                if (internalClock == -1) {
                    char hostName[1024];
                    gethostname(hostName, 1024);
                    string hostNameStr(hostName);

                    Computer comp = management.createComputer(discovery->getIPAddress(), discovery->getMacAddress());
                    comp.hostName = hostNameStr;
                    management.addComputer(comp);
                }
                threads.push_back(thread(&server, discovery.get()));    //TODO
                // threads.push_back(thread(&Discovery::server, discovery.get()));
                // threads.push_back(thread(&Monitoring::server, monitoring.get()));
                // threads.push_back(thread(&Interface::server, interface.get()));
            } 
            else {
                threads.push_back(thread(&client, discovery.get()));     //TODO
                // threads.push_back(thread(&Discovery::client, discovery.get()));
                // threads.push_back(thread(&Monitoring::client, monitoring.get()));
                // threads.push_back(thread(&Interface::client, interface.get()));
            }
        }
        catch (const exception& e) {
            cerr << "Exception: " << e.what() << endl;
            return 1;
        } catch (...) {
            cerr << "Unknown exception occurred" << endl;
            return 1;
        }
    }

    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    
    return 0;
}
