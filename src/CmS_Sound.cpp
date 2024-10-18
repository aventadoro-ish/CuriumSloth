#include "CmS_Sound.h"
#pragma comment(lib, "Ws2_32.lib")	   // Make sure we are linking against the Ws2_32.lib library
#pragma comment(lib, "Winmm.lib")      // Make sure we are linking against the Winmm.lib library - some functions/symbols from this library (Windows sound API) are used
#include <mmsystem.h>					
#include <math.h>

#include <iostream>

using namespace std;

void AudioRecorder::setupFormat(
	uint32_t numChannels, 
	uint32_t samplesPerSecond,
	uint16_t bitsPerSample) {

	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nChannels = numChannels;
	waveFormat.nSamplesPerSec = samplesPerSecond;
	waveFormat.wBitsPerSample = bitsPerSample;
	waveFormat.nBlockAlign = waveFormat.nChannels * (waveFormat.wBitsPerSample / 8);
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;
	return;

}

int AudioRecorder::initializeRecording() {
	MMRESULT rc;
	
	// open the recording device
	rc = waveInOpen(&hWaveIn, WAVE_MAPPER, &waveFormat, (DWORD)NULL, 0, CALLBACK_NULL);
	if (rc) {
		cout << "Unable to open Input sound Device! Error: " << rc << endl;
		return(-1);
	}

	// prepare the buffer header for use later on
	waveHeaderIn.lpData = (char*)recBuf;
	waveHeaderIn.dwBufferLength = recBufSize * sizeof(short);
	rc = waveInPrepareHeader(hWaveIn, &waveHeaderIn, sizeof(WAVEHDR));
	if (rc) {
		cout << "Failed preparing input WAVEHDR, error: " << rc << endl;
		return(-1);
	}

	return(0);
}

int AudioRecorder::recordBuffer() {
	MMRESULT	mmErr;
	int		rc;

	printf("Recording now.....");

	// stop previous recording (just in case)
	waveInReset(hWaveIn);   // is this good?

	rc = waveInPrepareHeader(hWaveIn, &waveHeaderIn, sizeof(WAVEHDR));
	if (rc) {
		cout << "Failed preparing WAVEHDR, error: " << rc << endl;
		return(0);
	}
	waveHeaderIn.dwFlags &= ~(WHDR_BEGINLOOP | WHDR_ENDLOOP);

	// Give the buffer to the recording device to fill.
	mmErr = waveInAddBuffer(hWaveIn, &waveHeaderIn, sizeof(WAVEHDR));
	// Record the buffer. This is NON-blocking.
	mmErr = waveInStart(hWaveIn);
	// wait for completion
	rc = waitOnHeader(&waveHeaderIn, '.');
	// stop input
	waveInStop(hWaveIn);
	return(rc);

}

int AudioRecorder::recordAudio(uint32_t seconds, uint32_t samplesPerSecond) {
	if (recBuf != nullptr) {
		cout << "ERROR! Trying to record to a buffer that is occupied!" << endl;
		return -1;
	}

	recLength = seconds;
	recSamplesPerSencod = samplesPerSecond;
	recBufSize = recLength * recSamplesPerSencod;


	// allocate space for the recording buffer
	recBuf = (short*)malloc(recBufSize);
	if (recBuf == nullptr) {
		cout << "ERROR! Unable to allocate the recording buffer!" << endl;
	}


	int res = recordBuffer();
	if (res != 0) {
		cout << "ERROR! recordBuffer() failed inside recordAudio(..)" << endl;
		return -1;
	}

	return 0;
}


int AudioRecorder::waitOnHeader(WAVEHDR* wh, char cDit) {
	long	lTime = 0;
	// wait for whatever is being played, to finish. Quit after 10 seconds.
	for (; ; ) {
		if (wh->dwFlags & WHDR_DONE) return(0);
		// idle for a bit so as to free CPU
		Sleep(100L);
		lTime += 100;
		if (lTime >= 10000) return(-1);  // timeout period
		if (cDit) printf("%c", cDit);
	}
}
