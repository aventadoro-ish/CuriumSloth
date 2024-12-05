/* RS232test.cpp - Updated Client for the Tx/Rx Program */
#include "RS232test.h"

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include "FortuneCookies.h"
#include "COMPort.h"

using namespace std;

// Constants
const int BUFSIZE = 200;  // Buffer size

// Externally defined variables for dynamic COM ports
extern std::string txPortName;
extern std::string rxPortName;

// COM Port settings
const COMPortBaud BAUD_RATE = COMPortBaud::COM_BAUD_460800;
const CPParity PARITY = CPParity::NONE;
const int STOP_BITS = 1;

template <typename T>
void populateQueue(Queue<T>& quoteQueue) {
    int numQuotes = fnumQuotes();
    if (numQuotes <= 0) {
        printf("Error: No quotes found in the file or file missing.\n");
        return;
    }

    long int* quoteIndices = fquoteIndices(numQuotes);
    int* quoteLengths = fquoteLength(numQuotes, quoteIndices);


    if (quoteIndices && quoteLengths) {
        srand((unsigned int)time(NULL));  // Seed random number generator

        for (int i = 0; i < 10; ++i) {  // Add 10 quotes to the queue
            int randIndex = frandNum(1, numQuotes);
            char buffer[BUFSIZE];
            int result = GetMessageFromFile(buffer, BUFSIZE, randIndex, numQuotes, quoteIndices, quoteLengths);
            if (result == 0) {
                string quote(buffer);
                Node<T>* newNode = new Node<T>{ nullptr, quote };
                quoteQueue.addToQueue(newNode);
            }
        }
    }

    free(quoteIndices);
    free(quoteLengths);
}

int rs232test() {
    // Ensure COM ports are set
    if (txPortName.empty() || rxPortName.empty()) {
        cerr << "Error: COM ports not set. Please set COM ports before testing.\n";
        return -1;
    }

    // Initialize the queue
    Queue<string> quoteQueue;
    quoteQueue.inItQueue();

    populateQueue(quoteQueue);

    // Initialize COM ports
    COMPort comRx(BAUD_RATE, PARITY, STOP_BITS);
    COMPort comTx(BAUD_RATE, PARITY, STOP_BITS);

    if (comRx.openPort((char*)rxPortName.c_str()) != CPErrorCode::SUCCESS) {
        cerr << "Failed to open Rx port: " << rxPortName << endl;
        return -1;
    }

    if (comTx.openPort((char*)txPortName.c_str()) != CPErrorCode::SUCCESS) {
        cerr << "Failed to open Tx port: " << txPortName << endl;
        comRx.closePort();
        return -1;
    }

    // Dequeue a quote for transmission
    if (quoteQueue.isQueueEmpty()) {
        cerr << "No quotes available to send!\n";
        comRx.closePort();
        comTx.closePort();
        return -1;
    }
    Node<string>* quoteNode = quoteQueue.deQueue();
    string msgOut = quoteNode->Data;
    delete quoteNode;

    // Transmit the message
    cout << "\nSending Message:\n" << msgOut << "\n";
    if (comTx.sendMessage((void*)msgOut.c_str(), msgOut.length() + 1) != CPErrorCode::SUCCESS) {
        cerr << "Failed to send message.\n";
    }

    // Receive a response (if any)
    char msgIn[BUFSIZE] = { 0 };
    if (comRx.receiveMessage((void*)msgIn, BUFSIZE, 0, 5000) == CPErrorCode::SUCCESS) {
        cout << "\nMessage Received:\n" << msgIn << "\n";
    }
    else {
        cerr << "Failed to receive message.\n";
    }

    // Clean up
    comRx.closePort();
    comTx.closePort();

    system("pause");
    return 0;
}