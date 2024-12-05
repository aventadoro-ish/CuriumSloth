/* COMPortManager.cpp : Set up COM Port
 * Date: Dec 2024
 * Author:
 *         Gobind Matharu
 */

#include "COMPortManager.h"
#include <iostream>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <dirent.h>
#endif

extern std::string txPortName; // Transmitter COM port
extern std::string rxPortName; // Receiver COM port

 // Convert std::string to std::wstring
std::wstring stringToWString(const std::string& str) {
    return std::wstring(str.begin(), str.end());
}

std::vector<std::string> listAvailableCOMPorts() {
    std::vector<std::string> ports;
#ifdef _WIN32
    wchar_t targetPath[5000]; // Wide-character array for QueryDosDevice compatibility
    for (int i = 1; i <= 256; ++i) {
        // Create the COM port name in wide string format
        std::string portName = "COM" + std::to_string(i);
        std::wstring widePortName = stringToWString(portName);

        // Query the COM port
        // TODO: Gobind, please verify the casts
        if (QueryDosDeviceW(widePortName.c_str(), targetPath, sizeof(targetPath) / sizeof(wchar_t))) {
            ports.push_back(portName); // Use narrow string for display/storage
        }
    }
#elif __linux__
    DIR* devDir = opendir("/dev");
    if (devDir) {
        struct dirent* entry;
        while ((entry = readdir(devDir)) != nullptr) {
            std::string name(entry->d_name);
            if (name.find("ttyS") == 0 || name.find("ttyUSB") == 0) {
                ports.push_back("/dev/" + name);
            }
        }
        closedir(devDir);
    }
#endif
    return ports;
}

std::string selectCOMPort() {
    auto ports = listAvailableCOMPorts();
    if (ports.empty()) {
        std::cerr << "No COM ports detected.\n";
        return "";
    }

    std::cout << "Available COM Ports:\n";
    for (size_t i = 0; i < ports.size(); ++i) {
        std::cout << i + 1 << ". " << ports[i] << "\n";
    }

    int choice;
    do {
        std::cout << "Select a COM port (1-" << ports.size() << "): ";
        std::cin >> choice;
    } while (choice < 1 || choice > static_cast<int>(ports.size()));

    return ports[choice - 1];
}

void removePort(std::vector<std::string>& ports, const std::string& portToRemove) {
    auto it = std::remove(ports.begin(), ports.end(), portToRemove);
    if (it != ports.end()) {
        ports.erase(it, ports.end());
    }
}


// Set the COM ports for transmitter and receiver
void setCOMport() {
    // List available COM ports
    std::vector<std::string> availablePorts = listAvailableCOMPorts();

    if (availablePorts.empty()) {
        std::cerr << "No COM ports detected. Exiting program.\n";
        exit(1); // Exit if no ports are available
    }

    // Display available COM ports
    std::cout << "Available COM Ports:\n";
    for (size_t i = 0; i < availablePorts.size(); ++i) {
        std::cout << i + 1 << ". " << availablePorts[i] << "\n";
    }

    // Select transmitter COM port
    int txChoice;
    do {
        std::cout << "Select Transmitter COM Port (1-" << availablePorts.size() << "): ";
        std::cin >> txChoice;
    } while (txChoice < 1 || txChoice > static_cast<int>(availablePorts.size()));

    // Store selected transmitter port
    txPortName = availablePorts[txChoice - 1];

    // Remove the selected transmitter port
    availablePorts.erase(availablePorts.begin() + (txChoice - 1));

    // Check if there are any ports left for receiver
    if (availablePorts.empty()) {
        std::cerr << "Only one COM port available. Cannot proceed.\n";
        exit(1); // Exit if no ports remain
    }

    // Display available COM ports again for receiver selection
    std::cout << "Available COM Ports for Receiver:\n";
    for (size_t i = 0; i < availablePorts.size(); ++i) {
        std::cout << i + 1 << ". " << availablePorts[i] << "\n";
    }

    // Select receiver COM port
    int rxChoice;
    do {
        std::cout << "Select Receiver COM Port (1-" << availablePorts.size() << "): ";
        std::cin >> rxChoice;
    } while (rxChoice < 1 || rxChoice > static_cast<int>(availablePorts.size()));

    // Store selected receiver port
    rxPortName = availablePorts[rxChoice - 1];

    // Display the selected ports
    std::cout << "\nTransmitter Port: " << txPortName << "\n";
    std::cout << "Receiver Port: " << rxPortName << "\n";
}
