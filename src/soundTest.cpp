/*	Filename: main.cpp
Author: Michael Galle
Date: Updated 2022
Details: Testing mainline for Windows sound API
*/

#include <stdio.h>
#include <windows.h>
#include <iostream>

#include "CmS_Sound.h"



int	soundTest(int argc, char *argv[]) {

	AudioRecorder aRec = AudioRecorder(44200, 16);
	std::cout << "Recording audio for 10 seconds: " << std::endl;
	aRec.recordAudio(10);											// record for 10 seconds
	short* buf = aRec.getBuffer();
	uint32_t bufLen = aRec.getBufferSize();


	short* newBuf = (short*)malloc(bufLen * sizeof(short));
	memcpy(newBuf, buf, bufLen * sizeof(short));


	AudioRecorder aReplayer = AudioRecorder(44200, 16);
	aReplayer.setBuffer(newBuf, bufLen);
	std::cout << "Replaying the recorded audio: " << std::endl; 
	aReplayer.replayAudio();

	free(newBuf);

	// Save audio to a file
	char save;
	FILE* f;

	std::cout << "Would you like to save your audio recording? (y/n): ";
	std::cin >> save;

	if (save == 'y' || save == 'Y') {
        fopen_s(&f, "C:\\myfiles\\recording.dat", "wb");
        if (!f) {
            std::cerr << "Unable to open C:\\myfiles\\recording.dat\n";
            return 0;
        }
        std::cout << "Writing to sound file ...\n";
        fwrite(buf, sizeof(short), bufLen, f);
        fclose(f);
    }

    // Replay audio from file
    char replay;
    std::cout << "Would you like to replay the saved audio recording from the file? (y/n): ";
    std::cin >> replay;

    if (replay == 'y' || replay == 'Y') {
        fopen_s(&f, "C:\\myfiles\\recording.dat", "rb");
        if (!f) {
            std::cerr << "Unable to open C:\\myfiles\\recording.dat\n";
            return 0;
        }

        // Allocate memory for a new buffer to read the saved data
        short* fileBuf = (short*)malloc(bufLen * sizeof(short));
        fread(fileBuf, sizeof(short), bufLen, f);
        fclose(f);

        // Play the sound from the file buffer
        aReplayer.setBuffer(fileBuf, bufLen);
        std::cout << "Replaying audio from the saved file: " << std::endl;
        aReplayer.replayAudio();

        // Free the file buffer
        free(fileBuf);
    }

    std::cout << "\n";
    system("pause");
    return 0;
}