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
#include "terminal.h"  // Include terminal-related functions

using namespace std;

// Function prototypes for non-terminal logic
void recordAudio();
void playAudio();
void queueMessage();
void displayQueue();
void communicationSettings();

// Global message queue definition
queue<string> messageQueue;

int main() {
    int choice;
    bool running = true;

    while (running) {
        displayMenu(); // Show the menu (from terminal.h)
        choice = getChoice(); // Get the user's choice (from terminal.h)

        switch (choice) {
            case 1:
                recordAudio();
                break;
            case 2:
                playAudio();
                break;
            case 3:
                queueMessage();
                break;
            case 4:
                displayQueue();
                break;
            case 5:
                communicationSettings();
                break;
            case 0:
                running = false; // Exit the loop
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
    printf("Recording audio (placeholder)...\n");
    // In actual implementation, you'd capture audio here.
}

// Placeholder function for playing back audio
void playAudio() {
    printf("Playing audio (placeholder)...\n");
    // In actual implementation, you'd play audio here.
}

// Function to queue a message
void queueMessage() {
}

// Function to display queued messages
void displayQueue() {
}

// Placeholder for communication settings function
void communicationSettings() {
    printf("Adjust communication settings (placeholder)...\n"); // In actual implementation, you can modify settings like bitrate, etc.
}
