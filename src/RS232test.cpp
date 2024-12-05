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

// Physical ports (adjust as necessary)
const char* COMPORT_Rx = "COM3";
const char* COMPORT_Tx = "COM4";

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
    // Initialize the queue
    Queue<string> quoteQueue;
    quoteQueue.inItQueue();

    populateQueue(quoteQueue);

    // Initialize COM ports
    COMPort comRx(BAUD_RATE, PARITY, STOP_BITS);
    COMPort comTx(BAUD_RATE, PARITY, STOP_BITS);

    if (comRx.openPort((char*)COMPORT_Rx) != CPErrorCode::SUCCESS) {
        cerr << "Failed to open Rx port: " << COMPORT_Rx << endl;
        return -1;
    }

    if (comTx.openPort((char*)COMPORT_Tx) != CPErrorCode::SUCCESS) {
        cerr << "Failed to open Tx port: " << COMPORT_Tx << endl;
        return -1;
    }

    // Dequeue a quote for transmission
    if (quoteQueue.isQueueEmpty()) {
        printf("No quotes available to send!\n");
        return -1;
    }
    Node<string>* quoteNode = quoteQueue.deQueue();
    string msgOut = quoteNode->Data;
    delete quoteNode;

    // Transmit the message
    printf("\nSending Message: \n%s\n", msgOut.c_str());
    if (comTx.sendMessage((void*)msgOut.c_str(), msgOut.length() + 1) != CPErrorCode::SUCCESS) {
        cerr << "Failed to send message." << endl;
    }

    // Receive a response (if any)
    char msgIn[BUFSIZE] = { 0 };
    if (comRx.receiveMessage((void*)msgIn, BUFSIZE) == CPErrorCode::SUCCESS) {
        printf("\nMessage Received: \n%s\n", msgIn);
    }
    else {
        cerr << "Failed to receive message." << endl;
    }

    // Clean up
    comRx.closePort();
    comTx.closePort();

    system("pause");
    return 0;
}
