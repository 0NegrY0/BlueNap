#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <netdb.h>

char* getIPAddress() {
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char *host = NULL;

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    // Walk through linked list, maintaining head pointer so we can free list later
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) {
            continue;
        }

        family = ifa->ifa_addr->sa_family;

        if (family == AF_INET || family == AF_INET6) {
            char addr[INET6_ADDRSTRLEN];
            s = getnameinfo(ifa->ifa_addr, (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
                            addr, INET6_ADDRSTRLEN, NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                std::cerr << "getnameinfo() failed: " << gai_strerror(s) << std::endl;
                exit(EXIT_FAILURE);
            }
            if (strcmp(ifa->ifa_name, "lo") != 0) {
                host = strdup(addr);
                break; // Found a non-loopback interface, no need to iterate further
            }
        }
    }

    freeifaddrs(ifaddr);
    return host;
}

int main() {
    char *ipAddress = getIPAddress();
    if (ipAddress != NULL) {
        std::cout << "IP Address: " << ipAddress << std::endl;
        free(ipAddress);
    } else {
        std::cerr << "Unable to retrieve IP address." << std::endl;
    }
    return 0;
}
