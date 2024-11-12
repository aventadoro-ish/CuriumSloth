/* terminal.cpp: header file for terminal.cpp
 * Date: Oct 2024
 * Author:  Gobind Matharu
 */

#include "terminal.h"

 // Function to display the main menu (Home/Receive/Exit)
void displayMainMenu() {
    printf("\n====== Main Menu ======\n");
    printf("1. Home\n");
    printf("2. Receive\n");
    printf("3. Communication Settings\n");
    printf("0. Exit\n");
    printf("=======================\n");
    printf("Enter your choice: ");
}

// Function to display the home menu (after choosing Home)
void displayHomeMenu() {
    printf("\n====== Home Menu ======\n");
    printf("1. Record Audio\n");
    printf("2. Play Audio\n");
    printf("3. Queue a Message\n");
    printf("4. Display Queued Messages\n");
    printf("0. Back\n");
    printf("========================\n");
    printf("Enter your choice: ");
}

// Function to display the receive menu (after choosing Receive)
void displayReceiveMenu() {
    printf("\n====== Receive Menu ======\n");
    printf("1. Check Incoming Messages\n");
    printf("2. Download Messages\n");
    printf("3. Manage Downloads\n");
    printf("0. Back\n");
    printf("==========================\n");
    printf("Enter your choice: ");
}

void displayCommunicationSettings() {
	printf("\n====== Communication Settings ======\n");
	printf("1. Adjust Bitrate\n");
	printf("2. Set COM Port\n");
	printf("3. Set Sample Rate\n");
	printf("0. Back\n");
	printf("====================================\n");
	printf("Enter your choice: ");
}


#if defined(_WIN32)
// Function to get the user's choice
int getChoice() {
    int choice;

#ifdef _MSC_VER // for Visual Studio
    scanf_s("%d", &choice);
#else           // for VS Code - scanf_s does not exist in g++.exe
    scanf("%d", &choice);
#endif
    return choice;
}
#elif defined(__linux__)

// Function to get the user's choice
int getChoice() {
    int choice;
    scanf("%d", &choice);
    return choice;
}
#else
// Function to get the user's choice
int getChoice() {
    int choice;
    scanf_s("%d", &choice);
    return choice;
}
#endif