/* COMPortTest.cpp : Tests the COMPort 
 * Date: Nov 2024
 * Author:
 *         Gobind Matharu
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <Windows.h>
#include "COMPortTest.h"
#include "utils.h"

using namespace std;

void testCOMPort(const char* txPortName, const char* rxPortName) {
    // Initialize two COM ports
    COMPort txPort(COMPortBaud::COM_BAUD_9600, CPParity::NONE, 1);
    COMPort rxPort(COMPortBaud::COM_BAUD_9600, CPParity::NONE, 1);

    // Open the transmitter port
    cout << "Opening transmitter port: " << txPortName << endl;
    if (txPort.openPort((char*)txPortName) != CPErrorCode::SUCCESS) {
        cerr << "Error: Failed to open transmitter port: " << txPortName << endl;
        return;
    }

    // Open the receiver port
    cout << "Opening receiver port: " << rxPortName << endl;
    if (rxPort.openPort((char*)rxPortName) != CPErrorCode::SUCCESS) {
        cerr << "Error: Failed to open receiver port: " << rxPortName << endl;
        txPort.closePort();
        return;
    }

    // Check if ports are open
    if (txPort.isPortOpen() && rxPort.isPortOpen()) {
        cout << "Success: Both ports are open." << endl;
    }
    else {
        cerr << "Error: One or both ports failed to open." << endl;
        txPort.closePort();
        rxPort.closePort();
        return;
    }

    // Transmit a test message
    const char* testMessage = "Hello from TX!";
    if (txPort.canWrite()) {
        cout << "Transmitting message: " << testMessage << endl;
        if (txPort.sendMessage((void*)testMessage, strlen(testMessage) + 1) == CPErrorCode::SUCCESS) {
            cout << "Success: Message transmitted." << endl;
        }
        else {
            cerr << "Error: Failed to transmit message." << endl;
        }
    }
    else {
        cerr << "Error: Cannot write to TX port (timeout in effect)." << endl;
    }

    // Allow some time for the message to reach RX
    Sleep(50);
    //std::this_thread::sleep_for(chrono::milliseconds(100));

    // Check for data in RX input buffer
    unsigned int rxInputBytes = rxPort.numInputBytes();
    cout << "RX input buffer size: " << rxInputBytes << " bytes." << endl;

    if (rxInputBytes > 0) {
        char rxBuffer[256] = { 0 }; // Buffer to receive data
        if (rxPort.receiveMessage(rxBuffer, sizeof(rxBuffer)) == CPErrorCode::SUCCESS) {
            cout << "Success: Message received: " << rxBuffer << endl;
        }
        else {
            cerr << "Error: Failed to receive message." << endl;
        }
    }
    else {
        cerr << "Error: No data received in RX input buffer." << endl;
    }

    // Close the ports
    cout << "Closing COM ports..." << endl;
    if (txPort.closePort() == CPErrorCode::SUCCESS) {
        cout << "TX port closed successfully." << endl;
    }
    else {
        cerr << "Error: Failed to close TX port." << endl;
    }

    if (rxPort.closePort() == CPErrorCode::SUCCESS) {
        cout << "RX port closed successfully." << endl;
    }
    else {
        cerr << "Error: Failed to close RX port." << endl;
    }
}
