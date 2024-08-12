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
    cout << "Tamanho" << computers.size();

    while (isMaster){

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
        "........-=======:..+#%#*-..========:.........\n"
        "................................BlueNapServer\n" << endl;

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

        string input; 
        cin >> input;
        if (input == "1"){
            cout << "Enter the ID of the client you want to awake: ";
            cin >> input;

            int id = stoi(input);
            cout << "Vou acordar o computador de id: " << id << endl;
            for (auto& c : computers){
                if (c.id == id){
                    management.wakeOnLan(c.macAddress, c.ipAddress);
                    sleep(1);
                }
            }         
        }
        
        system("clear");
    }
    return 0;
}


int Interface::client() {
    Management management;
    sleep(1);
    // Testar isso ai 
    while (!shouldExit && !isMaster){
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
        "........-=======:..+#%#*-..========:.........\n"
        "................................BlueNapClient\n" << endl;
        mtx.lock();
        cout << endl << "================ Computers in Network ===============" << endl;
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
            else{
                cout << endl << "============\\/ Leader Machine \\/============" << endl;
                cout << "ID: "<<computers[i].id<<"\t\tHostname: "<<computers[i].hostName<<"\t\tMAC Adress:"<<computers[i].macAddress<<"\t\tIP Adress: "<<computers[i].ipAddress;
                cout << endl << "============/\\ Leader Machine /\\============" << endl;
            }        
        }
        mtx.unlock();
        
        cout << endl << "You are a Client" << endl; 
        cout << "Enter 'EXIT' to leave, Enter anything to update" << endl;

        string input; 
        getline(cin, input);
            if (input == "EXIT"){
            management.askToCloseConnection();
            sleep(1);
            mtx.lock();
            shouldExit = true;
            mtx.unlock();
            }      
        
        system("clear");
    }
    return 0;
}

