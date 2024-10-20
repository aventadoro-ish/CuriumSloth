 /* terminal.cpp: header file for terminal.cpp
  * Date: Oct 2024
  * Version: 1.0
  * Author:  Gobind Matharu
  */
#include "terminal.h"

// Function to display the menu
void displayMenu() {
    printf("\n====== Coded Messaging System ======\n");
    printf("1. Record Audio\n");
    printf("2. Play Audio\n");
    printf("3. Queue a Message\n");
    printf("4. Display Queued Messages\n");
    printf("5. Communication Settings\n");
    printf("0. Exit\n");
    printf("====================================\n");
    printf("Enter your choice: ");
}

// Function to get the user's choice
int getChoice() {
    int choice;
    scanf_s("%d", &choice);
    return choice;
}
