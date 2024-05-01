#include "Headers/GetMacAdress.hpp"

namespace fs = std::filesystem;

std::string getMacAddress() {
    for (const auto& entry : fs::directory_iterator("/sys/class/net/")) {
        std::string interface = entry.path().filename();

        std::ifstream file("/sys/class/net/" + interface + "/address");
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string macAddress = buffer.str();

            // Remove trailing newline
            if (!macAddress.empty() && macAddress[macAddress.length() - 1] == '\n') {
                macAddress.erase(macAddress.length() - 1);
            }

            if (!macAddress.empty()) {
                return macAddress;
            }
        }
    }

    throw std::runtime_error("Failed to find a network interface with a MAC address.");
}

int main() {
    try {
        std::string macAddress = getMacAddress();
        std::cout << "MAC Address: " << macAddress << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}