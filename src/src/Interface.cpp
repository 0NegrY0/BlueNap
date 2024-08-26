#include "../include/Interface.hpp"
#include "../include/Management.hpp"
#include <iostream>
#include <string>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <cstdlib>
#include <future>
#include <chrono>

using namespace std;

bool getInputWithTimeout(string& input, int timeoutSeconds) {
    auto future = async(launch::async, []() {
        string temp;
        getline(cin, temp);
        return temp;
    });

    if (future.wait_for(chrono::seconds(timeoutSeconds)) == future_status::timeout) {
        return false;
    } else {
        input = future.get();
        return true;
    }
}

int Interface::server() {
    Management management;

    int index = 0;
    for (size_t i=0; i<computers.size(); i++){
        if (computers[i].isServer){
            index = i;
            break;
        }
    }

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
                cin >> input;

                int id = stoi(input);
                for (auto& c : computers){
                    if (c.id == id) {
                        management.wakeOnLan(c.macAddress);
                        sleep(1);
                    }
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
        cout << endl << "============ Leader Machine ============" << endl;
        mtx.lock();
        for (size_t i=0; i<computers.size(); i++){
            if (computers[i].isServer){
                cout << "ID: "<<computers[i].id<<"\t\tHostname: "<<computers[i].hostName<<"\t\tMAC Adress:"<<computers[i].macAddress<<"\t\tIP Adress: "<<computers[i].ipAddress;
            }
        }
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
        
        cout << endl << "You are a Client" << endl; 
        cout << "Enter 'EXIT' to leave, Enter anything to update" << endl;

        string input; 
        
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);

        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        int ret = select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv);

        if (ret > 0){
            string input; 
            getline(cin, input);
            if (input == "EXIT"){
                management.askToCloseConnection();
                sleep(1);
                mtx.lock();
                shouldExit = true;
                mtx.unlock();
            }      
        }
        system("clear");
    }
    cout << "Desligando interface cliente" << endl;
    return 0;
}