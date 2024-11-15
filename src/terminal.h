/* terminal.h: header file for terminal.cpp
 * Date: Oct 2024
 * Author:  Gobind Matharu
 */

#ifndef TERMINAL_H
#define TERMINAL_H

#include <cstdio>  // For printf and scanf
#include <stdio.h> // For scanf_s
#include <stdlib.h>

 // Function prototypes for terminal operations
void displayMainMenu();
void displayHomeMenu();
void displayReceiveMenu();
void displayCommunicationSettings();
int getChoice();
void clearScreen();

#endif // TERMINAL_H

