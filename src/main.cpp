#include <stdin>
#include <Discovery/Discovery_Subservice.h>
#include <Monitoring/Monitoring_Subservice.h>

using namespace std;

int main() {
    int isMaster;

    cout << "Are you the master? (1/0): ";
    cin >> isMaster;

    vector<thread> threads;

    if (isMaster) {
        threads.push_back(thread(handleDiscoveryReceiver));
        threads.push_back(thread(handleMonitoringSender));
    }
    else {
        threads.push_back(thread(handleDiscoverySender));
        threads.push_back(thread(handleMonitoringReceiver));
    }

    return 0;
}