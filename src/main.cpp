/* main.cpp : terminal based ui for a messaging system
 * Date: Oct 2024
 * Author:
 *         Mat Regentov
 *         Besart Kalezic
 *         Gobind Matharu
 */

#include <iostream>
#include <queue>
#include <string>
#include "terminal.h"
#include "CmS_Sound.h"
#include "COMPort.h"
#include "COMPortTest.h"
#include "COMPortManager.h"
#include "RS232test.h"
#include "Queue.h"
#include "Message.h"
#include "MessageManager.h"
#include "phonebook.h"

#ifdef _WIN32

#include <conio.h>

#endif

using namespace std;

std::string txPortName; // Transmitter port set at startup
std::string rxPortName; // Receiver port set at startup

// Function prototypes for non-terminal logic
void displayMainMenu();
void displayHomeMenu();
void displayReceiveMenu();
void displayCommunicationSettings();
void recordAudio();
void playAudio();
void queueMessage();
void displayQueue();
void communicationSettings();
void checkIncomingMessages();
void downloadMessages();
void manageDownloads();
void adjustBitrate();
void setSampleRate();
void rs232message();
void comPortTest();
void phonebookMenu();

int main() {
    int mainChoice, homeChoice, receieveChoice, communicationChoice;
    bool running = true;

    setCOMport();

    while (running) {
        displayMainMenu();  // Show the main menu (Home/Receive/Exit)
        mainChoice = getChoice();  // Get the user's choice from the main menu

        switch (mainChoice) {
        case 1: {  // Home selected
            bool inHomeMenu = true;
            while (inHomeMenu) {
                displayHomeMenu();  // Show the Home menu
                homeChoice = getChoice();  // Get the user's choice from the Home menu

                switch (homeChoice) {
                case 1:
                    recordAudio();
                    break;
                case 2:
                    playAudio();  // Play Audio
                    break;
                case 3:
                    queueMessage();  // Queue a Message
                    break;
                case 4:
                    displayQueue();  // Display Queued Messages
                    break;
                case 5:
                    rs232message(); // Test Function for RS232 communication
                    break;
                case 6:
                    comPortTest(); // Test Function for COM Port
                    break;
                case 0:
                    inHomeMenu = false;  // Go back to the main menu
                    break;
                default:
                    printf("Invalid choice. Please try again.\n");
                }
            }
            break;
        }
        case 2: {  // Receive selected
            bool inReceiveMenu = true;
            while (inReceiveMenu) {
                displayReceiveMenu();  // Show the Receive menu
                receieveChoice = getChoice();  // Get the user's choice from the Receive menu

                switch (receieveChoice) {
                case 1:
                    checkIncomingMessages();  // Check Incoming Messages
                    break;
                case 2:
                    downloadMessages();  // Download Messages
                    break;
                case 3:
                    manageDownloads();  // Manage Downloads
                    break;
                case 0:
                    inReceiveMenu = false;  // Go back to the main menu
                    break;
                default:
                    printf("Invalid choice. Please try again.\n");
                }
            }
            break;
        }
        case 3: {  // Communication selected
            bool inCommsMenu = true;
            while (inCommsMenu) {
                displayCommunicationSettings();  // Show the Receive menu
                communicationChoice = getChoice();  // Get the user's choice from the Receive menu

                switch (communicationChoice) {
                case 1:
                    adjustBitrate();  // Adjust Bitrate
                    break;
                case 2:
                    setCOMport();  // Set COM Port
                    break;
                case 3:
                    setSampleRate();  // Set Sample Rate
                    break;
                case 0:
                    inCommsMenu = false;  // Go back to the main menu
                    break;
                default:
                    printf("Invalid choice. Please try again.\n");
                }
            }
            break;
        }
        case 4: // Phonebook selected
            phonebookMenu();
            break;
        case 0:
            running = false;  // Exit the loop and terminate the program
            printf("Exiting the program. Goodbye!\n");
            break;
        default:
            printf("Invalid choice. Please try again.\n");
        }
    }

    return 0;
}

// Placeholder function for recording audio
void recordAudio() {
    soundTest(); // Test function recording and playing back audio
}

// Placeholder function for playing back audio
void playAudio() {
    printf("Playing audio (placeholder)...\n");
    // In actual implementation, you'd play audio here.
}

// Function to queue a message
void queueMessage() {
    testQueue();
}   // In actual implementation, you'd add a message to the queue.

// Function to display queued messages
void displayQueue() {
    printf("Display queue (placeholder)...\n");
}

// Wrapper function for RS232 communication testing
void rs232message() {
    rs232test(); // Test Function for RS232 communication
}

// Placeholder for communication settings function
void communicationSettings() {
    printf("Adjust communication settings (placeholder)...\n");
    // In actual implementation, you can modify settings like bitrate, etc.
}

// Placeholder functions for the Receive submenu
void checkIncomingMessages() {
    printf("Checking for incoming messages (placeholder)...\n");
}

void downloadMessages() {
    printf("Downloading messages (placeholder)...\n");
}

void manageDownloads() {
    printf("Managing downloads (placeholder)...\n");
}

void adjustBitrate() {
    printf("Adjusting bitrate (placeholder)...\n");
}

void setSampleRate() {
    printf("Setting sample rate (placeholder)...\n");
}

// Test COM Port
void comPortTest() {
    if (txPortName.empty() || rxPortName.empty()) {
        std::cerr << "COM ports not set.\n";
        return;
    }
    testCOMPort(txPortName.c_str(), rxPortName.c_str());
}

// Function to display the phonebook menu
void phonebookMenu() {
    setCOMport();

    Phonebook pb;
    pb.menu();
}