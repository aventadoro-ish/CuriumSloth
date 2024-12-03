#include <iostream>
#include <thread>
#include <chrono>
#include "COMPortTest.h"

/* COMPortTest.cpp : Tests the COMPort 
 * Date: Nov 2024
 * Author:
 *         Gobind Matharu
 */

using namespace std;


void testCOMPort(const char* txPortName, const char* rxPortName) {
    COMPort txPort(COMPortBaud::COM_BAUD_9600, CPParity::NONE, 1);
    COMPort rxPort(COMPortBaud::COM_BAUD_9600, CPParity::NONE, 1);

    if (txPort.openPort((char*)txPortName) != CPErrorCode::SUCCESS ||
        rxPort.openPort((char*)rxPortName) != CPErrorCode::SUCCESS) {
        std::cerr << "Failed to open COM ports.\n";
        return;
    }

    const char* message = "Hello, COM Ports!";
    txPort.sendMessage((void*)message, strlen(message) + 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    char buffer[256] = { 0 };
    if (rxPort.receiveMessage(buffer, sizeof(buffer)) == CPErrorCode::SUCCESS) {
        std::cout << "Received: " << buffer << "\n";
    }

    txPort.closePort();
    rxPort.closePort();

	system("pause"); 
}
