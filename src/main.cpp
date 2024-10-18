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
	

	QueueGen<int> q;  // Create a queue of integers

	// Enqueue three elements into the queue
	q.enqueue(10);
	q.enqueue(20);
	q.enqueue(30);

	// Print the front element (should print 10)
	cout << "Front element: " << q.peek() << endl;

	// Dequeue the front element and print the new front (should print 20)
	q.dequeue();
	cout << "Front element after dequeue: " << q.peek() << endl;

	// Print the current size of the queue (should print 2)
	cout << "Queue size: " << q.getSize() << endl;

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