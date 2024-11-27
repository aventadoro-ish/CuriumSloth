/* RS232test.cpp - Client for the Tx/Rx Program
 * 
 *
 */

#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "RS232Comm.h"
#include "Queue.h"
#include "message.h"

// Note: Comment out the Tx or Rx sections below to operate in single sided mode

using namespace std;

// Declare constants, variables and communication parameters
const int BUFSIZE = 140;							// Buffer size

// Virtual Ports (via COM0COM) - uncomment to use (comment out the physical ports)
//wchar_t COMPORT_Rx[] = L"COM6";					// COM port used for Rx (use L"COM6" for transmit program)
//wchar_t COMPORT_Tx[] = L"COM7";					// COM port used for Tx (use L"COM7" for transmit program)

//Physical ports
wchar_t COMPORT_Rx[] = L"COM3";						// Check device manager after plugging device in and change this port
// wchar_t COMPORT_Tx[] = L"\\\\.\\COM10";				// Check device manager after plugging device in and change this port
wchar_t COMPORT_Tx[] = L"COM4";										// --> If COM# is larger than 9 then use the following syntax--> "\\\\.\\COM10"

// Communication variables and parameters
HANDLE hComRx;										// Pointer to the selected COM port (Receiver)
HANDLE hComTx;										// Pointer to the selected COM port (Transmitter)
int nComRate = 460800;								// Baud (Bit) rate in bits/second 9600
int nComBits = 8;									// Number of bits per frame
COMMTIMEOUTS timeout;								// A commtimeout struct variable

// Function to populate the queue with random quotes
template <typename T>
void populateQueue(Queue<T>& quoteQueue) {
    int numQuotes = fnumQuotes();
    if (numQuotes <= 0) {
        printf("Error: No quotes found in the file or file missing.\n");
        return;
    }

    long int* quoteIndices = fquoteIndices(numQuotes);
    int* quoteLengths = fquoteLength(numQuotes, quoteIndices);

        // Validate `quoteIndices` and `quoteLengths` here
        if (quoteIndices) {
            for (int i = 0; i < numQuotes; i++) {
                if (quoteIndices[i] < 0) {
                    printf("Error: Invalid quote index %ld at position %d\n", quoteIndices[i], i);
                }
            }
        }

        if (quoteLengths) {
            for (int i = 0; i < numQuotes; i++) {
                if (quoteLengths[i] < 0 || quoteLengths[i] > MAX_QUOTE_LENGTH) {
                    printf("Error: Invalid quote length %d at position %d\n", quoteLengths[i], i);
                }
            }
        }

        srand((unsigned int)time(NULL)); // Seed random number generator

        // Enqueue random quotes
        for (int i = 0; i < 10; ++i) { // Add 10 quotes to the queue as an example
            int randIndex = frandNum(1, numQuotes); // Random index between 1 and numQuotes
            char buffer[BUFSIZE];
            int result = GetMessageFromFile(buffer, BUFSIZE, randIndex, numQuotes, quoteIndices, quoteLengths);
            if (result == 0) {
                string quote(buffer);
                Node<T>* newNode = new Node<T>{ nullptr, quote }; // Create a new node
                quoteQueue.addToQueue(newNode); // Add to the queue
            }
        }

        free(quoteIndices); // Clean up memory
        free(quoteLengths); // Clean up memory
}

 // The client - A testing main that calls the functions below
int rs232test() {

	// Initialize the queue
	Queue<string> quoteQueue; // Create a queue of strings
	quoteQueue.inItQueue(); 

	populateQueue(quoteQueue); // Populate the queue with random quotes


    // Set up the COM ports
    initPort(&hComRx, COMPORT_Rx, nComRate, nComBits, timeout);
    Sleep(500);
    initPort(&hComTx, COMPORT_Tx, nComRate, nComBits, timeout);
    Sleep(500);

    // Dequeue a quote for transmission
    if (quoteQueue.isQueueEmpty()) {
        printf("No quotes available to send!\n");
        return -1;
    }
    Node<string>* quoteNode = quoteQueue.deQueue(); // Get a random quote
	string msgOut = quoteNode->Data; // Extract the message
    delete quoteNode; // Clean up the node

	// Transmit the message
    printf("\nSending Message: \n%s\n", msgOut.c_str());
    outputToPort(&hComTx, msgOut.c_str(), msgOut.length() + 1); 
    Sleep(500);

    // Receive a response (if any)
    char msgIn[BUFSIZE];
    DWORD bytesRead = inputFromPort(&hComRx, msgIn, BUFSIZE);
    msgIn[bytesRead] = '\0';
    printf("\nMessage Received: \n%s\n", msgIn);

	// Clean up
    purgePort(&hComRx);
    purgePort(&hComTx);
    CloseHandle(hComRx);
    CloseHandle(hComTx);

    system("pause");
    return 0;
}