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
#include "Queue.h"

using namespace std;

// Function prototypes for non-terminal logic
void displayMainMenu();
void displayHomeMenu();
void displayReceiveMenu();
void recordAudio();
void playAudio();
void queueMessage();
void displayQueue();
void communicationSettings();
void checkIncomingMessages();
void downloadMessages();
void manageDownloads();

// Global message queue definition
queue<string> messageQueue;

int main() {
    COMPort p1 = COMPort(COMPortBaud::BAUD_115200, CPParity::NONE, 2);
    COMPort p2 = COMPort(COMPortBaud::BAUD_115200, CPParity::NONE, 2);

    // p1.openPort("/dev/ttyUSB0");
    // p2.openPort("/dev/ttyUSB1");

    p1.openPort("/dev/pts/3");
    p2.openPort("/dev/pts/2");
    
    cout << "Here 1 " << endl;

    AudioRecorder ar = AudioRecorder(0x4000, 16, 2);
    ar.recordAudio(5);
    cout << "Here 2 " << endl;
    ar.replayAudio();

    return 0;
    // p1.sendMessage(ar.getBuffer(), ar.getBufferSize());
    cout << "Here 3 " << endl;

    // for (int i = 0; i < 32; i++) {
    //     cout << ar.getBuffer()[i];
    // }


    char* recBud = (char*)malloc(ar.getBufferSize() * sizeof(char));

    p2.receiveMessage(recBud, ar.getBufferSize());

    AudioRecorder ap = AudioRecorder(0x4000, 16, 2);
    ap.setBuffer((short*)recBud, ar.getBufferSize());
    // for (int i = 0; i < 32; i++) {
    //     cout << ap.getBuffer()[i];
    // }

    // ap.replayAudio();

    p1.closePort();
    p2.closePort();
    cout << "Here 4 " << endl;

    /* COM Port text mesasge demo */
    // char* msg = "Hello there!";
    // char* recMsg = (char*)malloc(100 * sizeof(char));
    // std::cout << p1.sendMessage(msg, strlen(msg)) << endl;
    // std::cout << p2.receiveMessage(recMsg, 100) << endl;
    // recMsg[strlen(recMsg)] = 0;
    // std::cout << recMsg << endl;

    return 0;

    int mainChoice, homeChoice, receieveChoice;
    bool running = true;

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
                    communicationSettings();  // Communication Settings
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
    soundTest();
    // In actual implementation, you'd capture audio here.
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
