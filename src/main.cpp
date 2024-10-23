/*	Filename: main.cpp
Author: Michael Galle
Date: Updated 2022
Details: Testing mainline for Windows sound API
*/

#include "sound.h"
#include "Queue.h"
#include <iostream>
#include <stdio.h>
#include <windows.h>

int	main(int argc, char *argv[]) //change to test audio bring it into diagnostics into sub menu .h and .cpp into program and test it out
{



	extern short iBigBuf[];												// buffer
	extern long  lBigBufSize;											// total number of samples
	short* iBigBufNew = (short*)malloc(lBigBufSize*sizeof(short));		// buffer used for reading recorded sound from file

	char save;
	char replay;
	char c;																// used to flush extra input
	FILE* f;
	
	//-----------------------------------Queue Code with Template
	Queue<int> q;
	Node<int>* p;

	// Initialize the queue
	q.inItQueue();

	// Add nodes with values from 1 to 10 to the queue
	for (int i = 1; i <= 10; i++) {
		p = new Node<int>();  // Allocate new node
		p->Data = i;          // Set the data of the node
		q.addToQueue(p);      // Add to the queue
	}

	// Print the queue before deletion
	std::cout << "Queue before deletion of 7:" << std::endl;
	q.traverse(q.returnHead(), visit);

	// Value to be deleted
	int valueToDelete = 7;

	// Call deleteR to remove the node with the value 7
	q.deleteR(q.returnHead(), q.returnHead()->pNext, valueToDelete);

	// Print the queue after deletion
	std::cout << "\nQueue after deletion of 7:" << std::endl;
	q.traverse(q.returnHead(), visit);

	p = new Node<int>();  // Allocate new node
	p->Data = 7;          // Set the data of the node
	q.addToQueue(p);
	std::cout << "\nQueue after adding of 7:" << std::endl;
	q.traverse(q.returnHead(), visit);

	//----------------------------------------Queue Code

	// initialize playback and recording
	InitializePlayback();
	InitializeRecording();

	// start recording
	RecordBuffer(iBigBuf, lBigBufSize);
	CloseRecording();

	// playback recording 
	printf("\nPlaying recording from buffer\n");
	PlayBuffer(iBigBuf, lBigBufSize);
	ClosePlayback();

	// save audio recording  
	printf("Would you like to save your audio recording? (y/n): "); 
	scanf_s("%c", &save, 1);
	while ((c = getchar()) != '\n' && c != EOF) {}								// Flush other input
	if ((save == 'y') || (save == 'Y')) {
		/* Open input file */
		fopen_s(&f, "C:\\myfiles\\recording.dat", "wb");
		if (!f) {
			printf("unable to open %s\n", "C:\\myfiles\\recording.dat");
			return 0;
		}
		printf("Writing to sound file ...\n");
		fwrite(iBigBuf, sizeof(short), lBigBufSize, f);
		fclose(f);
	}

	// replay audio recording from file -- read and store in buffer, then use playback() to play it
	printf("Would you like to replay the saved audio recording from the file? (y/n): ");
	scanf_s("%c", &replay, 1);
	while ((c = getchar()) != '\n' && c != EOF) {}								// Flush other input
	if ((replay == 'y') || (replay == 'Y')) {
		/* Open input file */
		fopen_s(&f, "C:\\myfiles\\recording.dat", "rb");
		if (!f) {
			printf("unable to open %s\n", "C:\\myfiles\\recording.dat");
			return 0;
		}
		printf("Reading from sound file ...\n");
		fread(iBigBufNew, sizeof(short), lBigBufSize, f);				// Record to new buffer iBigBufNew
		fclose(f);
		InitializePlayback();
		printf("\nPlaying recording from saved file ...\n");
		PlayBuffer(iBigBufNew, lBigBufSize);
		ClosePlayback();
	}
	
	printf("\n");
	system("pause");
	return(0);
}