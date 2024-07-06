#include "Headers/Interface_Subservice.hpp"
#include <iostream>
#include <string>
#include <cstring>
#include <cerrno>
#include <unistd.h>

using namespace std;

vector<Computer> computers; // Vector to store information of the computers


// Function to send discovery messages
void Interface() {
    bool isServer;

    int index = 0;
    Computer leader;
    leader = computers[index];
    while (!leader.isServer){
        index++;
        leader = computers[index];
    }
    string input;

    while (true){
        cout << endl << "========== Leader Machine ==========" << endl;
        cout << "ID: "<<leader.id<<" MAC Adress:"<<leader.macAddress<<" IP Adress: "<<leader.ipAddress;
        cout << endl << "========== Clients ==========" << endl;
        for (int i=0; i<(sizeof(computers) / sizeof(computers[0])); i++){
            if (i != index){
                cout << "ID: "<<computers[i].id<<" MAC Adress:"<<computers[i].macAddress<<" IP Adress: "<<computers[i].ipAddress<<endl;
            }
        }
        cout << "ID: "<<leader.id<<" MAC Adress:"<<leader.macAddress<<" IP Adress: "<<leader.ipAddress;
        if (isServer){
            cout << endl << "You are the Leader" << endl; 
            cout << "Enter 0 to exit, Enter 1 to wake a client, Enter anything to update" << endl; 
        }
        else {
            cout << endl << "You are a Cliente" << endl << "Enter 0 to exit, Enter anything to update" << endl; 
        }
        getline(cin, input);
        if (input == "0"){
            return;
        }
        if (input == "1" and isServer){
            cout << "Enter the ID of the client you want to awake: ";
            getline(cin, input);
            //acordar (input); FUNCAO DO TIAGO OU EVENTO SEILA
        }
        cout <<endl<<endl<<endl<<endl<<endl<<endl<<endl<<endl<<endl<<endl<<endl<<endl;
    }
    return;
}

int main() {
    std::vector<std::thread> threads;

    // Add threads to create interface
    threads.push_back(std::thread(Interface));

    // Wait for all threads to finish
    for (auto& th : threads) {
        th.join();
    }

    return 0;
}
