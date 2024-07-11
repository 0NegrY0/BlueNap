#include "../include/Interface.hpp"
#include "../include/Management.hpp"
#include <iostream>
#include <string>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <cstdlib>

using namespace std;

int Interface::server() {
    Management management;

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
        cout << endl << "You are the Leader" << endl; 
        cout << "Enter 1 to wake a client, Enter anything to update" << endl; 
        getline(cin, input);
        if (input == "1"){
            cout << "Enter the ID of the client you want to awake: ";
            getline(cin, input);
            int id = stoi(input);
            management.wakeOnLan(computers[id].macAddress, computers[id].ipAddress);
        }
        system("clear");
    }
    return 0;
}


int Interface::client() {
    string macAddress = getMacAddress();
    string ipAddress = getIPAddress();

    // Testar isso ai 
    
    cout <<"You are a Client" <<endl<<" MAC Adress:"<<macAddress<<" IP Adress: "<<ipAddress<<endl; 
    return 0;
}

