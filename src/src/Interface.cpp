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

    for (size_t i=0; i<computers.size(); i++){
        if (computers[i].isServer){
            index = i;
            break;
        }
    }

    while (true){

        cout << endl << "============ Leader Machine ============" << endl;
        mtx.lock();
        cout << "ID: "<<computers[index].id<<"\t\tHostname: "<<computers[index].hostName<<"\t\tMAC Adress:"<<computers[index].macAddress<<"\t\tIP Adress: "<<computers[index].ipAddress;
        cout << endl << "================ Clients ===============" << endl;
        for (size_t i=0; i<computers.size(); i++){
            if (!computers[i].isServer){
                cout << "ID: "<<computers[i].id<<"\t\tHostname: "<<computers[i].hostName<<"\t\tMAC Adress:"<<computers[i].macAddress<<"\t\tIP Adress: "<<computers[i].ipAddress<<"\t\tIs awake: ";
                if (computers[i].isAwake){
                    cout << "Yes"<<endl;
                }
                else{
                    cout << "No"<<endl;
                }
            }
        }
        mtx.unlock();

        cout << endl << "You are the Leader" << endl; 
        cout << "Enter 1 to wake a client, Enter anything to update" << endl;

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);

        struct timeval tv;
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        int ret = select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv);

        if (ret > 0){
            string input; 
            getline(cin, input);
            if (input == "1"){
                cout << "Enter the ID of the client you want to awake: ";
                getline(cin, input);

                size_t id = stoi(input);
                
                if (id < 2 || id > computers.size()){
                    cout << "ID invalido";
                }
                else{
                    management.wakeOnLan(computers[id - 1].macAddress, computers[id - 1].ipAddress);
                }
            }
        }
        system("clear");
    }
    return 0;
}


int Interface::client() {
    string macAddress = getMacAddress();
    string ipAddress = getIPAddress();
    char hostName[1024];
    gethostname(hostName, 1024);
    string input;

    // Testar isso ai 
    while (true){
        cout <<
        "........................=+++++...............\n"
        "........................@@@@@@-..............\n"
        "..........................#@@................\n"
        "...................----..@@*........@@@@@@:..\n"
        ".............=*@@@@@@@+.+@@@@@-.......-@@-...\n"
        "..........+#%@@*=---==........:**:...*@@.....\n"
        "........*#*+....................-**-=@@@@@:..\n"
        "......=*+...:*%%#-.......:*%%#=...-**........\n"
        ".....+*-..+@@@@@@@@*...-@@@@@@@@%...**:......\n"
        "....**:..@@@@@@@@@@@#.=@@@@@@@@@@@:..**......\n"
        "...=*:..%@@@@@@@@@@@@*@@@@@@@@@@@@@:..**.....\n"
        "...**..-@@@*:@@@@==@@@@@%:@@@@*:@@@*..-*-....\n"
        "..=*-..+@@@@..=+..%@@@@@@..=+..*@@@%...*+....\n"
        "..+*:..=@@@@@@@@@@+.-%+..@@@@@@@@@@#...**....\n"
        "..+*-...@@@@@@@@@@:.-++..%@@@@@@@@@-..:**:...\n"
        "..+****....:#@@@@@@@-..@@@@@@@@=....+****....\n"
        "..=*****##*....+@@@@@@@@@@@%:...+##*****+....\n"
        "...*********%%-..:@@@@@@@=...#%*********-....\n"
        "...=***********@#...@@@=..=@#**********+.....\n"
        "....=+***********%+..+..:%************+:.....\n"
        ".......-=.-*******#%...=%*******+.-=.........\n"
        "...+++*....-*******#:..%*******+....=*++:....\n"
        "...==++++....-++**+=...-+***+=....=+++==:....\n"
        "....======+=:........#:........=++=====......\n"
        "........-=======:..+#%#*-..========:.........\n" << endl;
        cout <<"You are a Client" <<endl<<"Hostname: "<<hostName<<"\t\t MAC Adress:"<<macAddress<<"\t\tIP Adress: "<<ipAddress<<endl;
        cout <<"Leader information:\t\tHostname: "<<serverHostName<<"\t\tMAC Adress"<<serverMac<<"\t\tIP Adress: "<<serverIp<<endl;
        cout <<"Enter 'EXIT' to leave"<<endl;
        getline(cin, input);
        if (input == "EXIT"){
            // FUNCAO CANCELAR O CPF
        }
        system("clear");
    }
    return 0;
}

