#ifndef MANAGEMENT_HPP
#define MANAGEMENT_HPP

#include "Utils.hpp"
#include <vector>
#include <mutex>
#include <condition_variable>

class Management : public Utils {

public:
    void addComputer(const Computer& computer);
    void removeComputer(int id);
    void updateStatus(int id, bool isAwake);
    vector<Computer> getComputers();
    void wakeOnLan(const string& macAddress, const string& ipAddress);
    void handleStatusUpdate(int id, bool isAwake);
    struct Computer createComputer(string clientIp, string clienMac);
    int getPort(int computerId);
    void startElection(int initiator);
    void announceElectionResult();
    void sendComputers(int socket, vector<Computer>& vec);
    vector<Computer>receiveComputers(int socket);
    Computer deserialize(const char* data, size_t& bytesRead);
    vector<char> setMonitoringMessage();
    void receiveComputers(vector<char>& buffer);
    Computer deserialize(const char* data, size_t& bytesRead);

private:
    vector<Computer> computers;
    mutable mutex mtx;
};

#endif