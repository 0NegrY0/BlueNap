#include "../include/Interface.hpp"
#include <iostream>
#include <string>
#include <cstring>
#include <cerrno>
#include <unistd.h>

using namespace std;

int Interface::server() {

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
            //acordar (input); FUNCAO DO TIAGO OU EVENTO SEILA
        }
        system(clear);
        //cout <<endl<<endl<<endl<<endl<<endl<<endl<<endl<<endl<<endl<<endl<<endl<<endl;
    }
    return 0;
}


int Interface::client() {
    string macAddress;
    string ipAddress;
    // Alguem botar aqui como faz pra pegar o endereco e salvar nas variavel :D
    //
    cout <<"You are a Client" <<endl<<" MAC Adress:"<<macAddress<<" IP Adress: "<<ipAddress<<endl; 

    //system(clear);
    //cout <<endl<<endl<<endl<<endl<<endl<<endl<<endl<<endl<<endl<<endl<<endl<<endl;
    return 0;
}

