/* COMPortManager.cpp : Set up COM Port
 * Date: Dec 2024
 * Author:
 *         Gobind Matharu
 */

#include "COMPortManager.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <dirent.h>
#endif

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
        if (QueryDosDevice(widePortName.c_str(), targetPath, sizeof(targetPath) / sizeof(wchar_t))) {
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

