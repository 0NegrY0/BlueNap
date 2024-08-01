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

int main(int agrc, char* agrv[]) {
    
    int isMaster;

    cout << "Are you the master? (1/0): ";
    cin >> isMaster;

    vector<thread> threads;
      
    // Usar unique_ptr para gerenciar a memória automaticamente
    unique_ptr<Discovery> discovery = make_unique<Discovery>();
    unique_ptr<Monitoring> monitoring = make_unique<Monitoring>();
    unique_ptr<Management> management = make_unique<Management>();
    unique_ptr<Interface> interface = make_unique<Interface>();
    
    while(true) {

        try {
            Management management;
            if (isMaster) {
                if (internalClock == -1) {
                    char hostName[1024];
                    gethostname(hostName, 1024);
                    string hostNameStr(hostName);

                    Computer comp = management.createComputer(discovery->getIPAddress(), discovery->getMacAddress())
                    comp.hostName = hostNameStr;
                    management.addComputer(comp);
                }
                
                threads.push_back(thread(&Discovery::server, discovery.get()));
                threads.push_back(thread(&Monitoring::server, monitoring.get()));
                threads.push_back(thread(&Interface::server, interface.get()));
            } 
            else {
                threads.push_back(thread(&Discovery::client, discovery.get()));
                threads.push_back(thread(&Monitoring::client, monitoring.get()));
                threads.push_back(thread(&Interface::client, interface.get()));
            }

            future<vector<Computer>> futureResult = async(launch::async, threadFunc);
            vector<Computer> result = futureResult.get();
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