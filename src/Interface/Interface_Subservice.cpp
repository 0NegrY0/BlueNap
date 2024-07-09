#include "Utils.hpp"
#include <iostream>
#include <string>
#include <cstring>
#include <cerrno>
#include <unistd.h>

using namespace std;

// Function to send discovery messages
void Interface() {
    
    int index = 0;
    while (!computers[index].isServer){
        index++;
    }
    string input;

    while (true){
        cout << endl << "========== Leader Machine ==========" << endl;
        cout << "ID: "<<computers[index].id<<" MAC Adress:"<<computers[index].macAddress<<" IP Adress: "<<computers[index].ipAddress;
        cout << endl << "========== Clients ==========" << endl;
        for (int i=0; i<computers.size(); i++){
            if (!computers[i].isServer){
                cout << "ID: "<<computers[i].id<<" MAC Adress:"<<computers[i].macAddress<<" IP Adress: "<<computers[i].ipAddress<<" Is awake: ";
                if (computers[i].isAwake){
                    cout << "Yes"<<endl;
                }
                else{
                    cout << "No"<<endl;
                }
            }
        }
        if (isServer){
            cout << endl << "You are the Leader" << endl; 
            cout << "Enter 0 to exit, Enter 1 to wake a client, Enter anything to update" << endl; 
        }
        else {
            cout << endl << "You are a Client" << endl << "Enter 0 to exit, Enter anything to update" << endl; 
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
        system(clear);
        //cout <<endl<<endl<<endl<<endl<<endl<<endl<<endl<<endl<<endl<<endl<<endl<<endl;
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
