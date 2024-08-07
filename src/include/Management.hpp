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

private:
    vector<Computer> computers;
    mutable mutex mtx;
};

#endif