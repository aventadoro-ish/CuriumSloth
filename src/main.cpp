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
#include "Message.h"
#include "MessageManager.h"

#include "linux_utils.h"


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
void adjustBitrate();
void setCOMport();
void setSampleRate();

// Global message queue definition
queue<string> messageQueue;

void recSide() {

    // Discards the input buffer
    fflush(stdin);


    COMPort port = COMPort();
    while(port.openPort("/dev/ttyUSB1") != CPErrorCode::SUCCESS) {
        ;
    }
    

    
    MessageManger mngr = MessageManger(1, 5);
    mngr.setCOMPort(&port);

    // while (mngr.tick()) {
    while (true) {
        mngr.tick();

        if (kbhit()) {
            cout << "exiting receive mode" << endl;
            break;
        }
    }


    // size_t buf_size = 500;
    // void* buf = (void*)malloc(buf_size);
    // for (;;) {
    //     if (port.receiveMessage(buf, buf_size) == CPErrorCode::SUCCESS) {
    //         Message msg = Message();
    //         msg.addData(buf, buf_size, false);
    //         msg.decodeMessage();
    //         // cout << (char*)msg.getMessage() << endl;
    //     } else {
    //         cout << "Port read error" << endl;
    //     }
    //     if (kbhit()) {
    //         cout << "Closing program" << endl;
    //         return;
    //     }
    // }

}

void sendSide() {
    // Discards the input buffer
    fflush(stdin);

    char* test1 = "Heeeeeeeello there my deeeeeeeeeearrrr friend!";

    COMPort port = COMPort();
    while(port.openPort("/dev/ttyUSB0") != CPErrorCode::SUCCESS) {
        ;
    }

    MessageManger mngr = MessageManger(0, 5);
    mngr.setCOMPort(&port);


    mngr.transmitData(1, MSGType::TEXT, test1, strlen(test1) + 1);


    cout << endl << endl << endl;


    while (mngr.tick()) {
        sleep(3);
        // for (unsigned int i = 0; i != 0xfffff; i++) {
        //     ; // wait
        // }
    }

    cout << "Sending ended with no isuuse" << endl;
}


int main() {
    int cmd;
    cin >> cmd;
    if (cmd == 0) {
        getchar();

        recSide();
    } else if (cmd == 1) {
        getchar();

        sendSide();
    } else {
        return 0;
    }



    return 0;

    char* test1 = "Heeeeeeeello there my deeeeeeeearrrr friend!";



    Message origin = Message();
    Message dst = Message();
    char* key = "encryption key!";

    // add raw payload data in encode mode (true - default)
    origin.addData(test1, strlen(test1) + 1); // + 1 accounts for the null-termination
    origin.describeData(0, 1, 0, MSGType::TEXT, MSGEncryption::XOR, MSGCompression::RLE);
    origin.setEncryptionKey(key, strlen(key) + 1);  // TODO: ADD ENCRYPTION SUPPORT
    origin.encodeMessage();     // generate header, payload, footer

    cout << endl << endl << endl;

    // transmission 

    // add received data in decode mode (false argument)
    dst.addData(origin.getMessage(), origin.getMessageSize(), false);
    dst.setEncryptionKey(key, strlen(key) + 1);  // TODO: ADD ENCRYPTION SUPPORT

    dst.decodeMessage();    // extract metadata and payload

    // use received and decoded data
    char* test2 = (char*)dst.getMessage();
    cout << test2 << endl;
    return 0;


    int mainChoice, homeChoice, receieveChoice, communicationChoice;
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

void setCOMport() {
	printf("Setting COM port (placeholder)...\n");
}   

void setSampleRate() {
	printf("Setting sample rate (placeholder)...\n");
}
