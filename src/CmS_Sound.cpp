#include "CmS_Sound.h"
#pragma comment(lib, "Ws2_32.lib")	   // Make sure we are linking against the Ws2_32.lib library
#pragma comment(lib, "Winmm.lib")      // Make sure we are linking against the Winmm.lib library - some functions/symbols from this library (Windows sound API) are used
#include <mmsystem.h>					
#include <math.h>

#include <iostream>

using namespace std;




AudioRecorder::AudioRecorder(
	uint32_t samplesPerSecond,
	uint16_t bitsPerSample,
	uint16_t numChannels) {

	recSamplesPerSencod = samplesPerSecond;
	recBitsPerSample = bitsPerSample;

}

AudioRecorder::~AudioRecorder() {
}

int AudioRecorder::recordAudio(uint32_t seconds) {
	if (recBuf != nullptr) {
		cerr << "ERROR! Trying to rewrite recBuf!" << endl;
		return -1;
	}

	recBufSize = seconds * recSamplesPerSencod;
	recBuf = (short*)malloc(recBufSize * sizeof(short));
	if (recBuf == nullptr) {
		cerr << "ERROR! Could not allocate memory for recBuf!" << endl;
		return -1;
	}


	initializeRecording();
	recordBuffer();
	closeRecordring();
	return 0;
}

int AudioRecorder::replayAudio() {
	initializePlayback();
	playBuffer();
	closePlayback();
	return 0;
}





void AudioRecorder::setupFormat() {
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nChannels = recNumCh;
	waveFormat.nSamplesPerSec = recSamplesPerSencod;
	waveFormat.wBitsPerSample = recBitsPerSample;
	waveFormat.nBlockAlign = waveFormat.nChannels * (waveFormat.wBitsPerSample / 8);
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;
	return;

}

int AudioRecorder::initializeRecording() {
	MMRESULT rc;

	// set up the format structure, needed for recording.
	setupFormat();

	// open the recording device
	rc = waveInOpen(&hWaveIn, WAVE_MAPPER, &waveFormat, (DWORD)NULL, 0, CALLBACK_NULL);
	if (rc) {
		printf("Unable to open Input sound Device! Error %x.", rc);
		return(0);
	}

	rc = waveInUnprepareHeader(hWaveIn, &waveHeaderIn, sizeof(WAVEHDR));

	// prepare the buffer header for use later on
	waveHeaderIn.lpData = (char*)recBuf;
	waveHeaderIn.dwBufferLength = recBufSize * sizeof(short) - 1;
	rc = waveInPrepareHeader(hWaveIn, &waveHeaderIn, sizeof(WAVEHDR));
	if (rc) {
		cerr << "Failed preparing input WAVEHDR, error x." << rc << endl;
		return(-1);
	}

	return(0);
}

int AudioRecorder::recordBuffer() {
	//static  WAVEHDR	WaveHeader;			/* WAVEHDR structure for this buffer */
	MMRESULT	mmErr;
	int		rc;

	printf("Recording now.....");

	// stop previous recording (just in case)
	waveInReset(hWaveIn);   // is this good?

	// get the header ready for recording.  This should not be needed here AND in init.
	//waveHeaderIn.lpData = (char*)recBuf;
	//waveHeaderIn.dwBufferLength = recBufSize * sizeof(short);
	//rc = waveInPrepareHeader(hWaveIn, &waveHeaderIn, sizeof(WAVEHDR));
	//if (rc) {
	//	cerr << "Failed preparing WAVEHDR, error x." << rc << endl;
	//	return(-1);
	//}
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

void AudioRecorder::closeRecordring() {
	waveInUnprepareHeader(hWaveIn, &waveHeaderIn, sizeof(WAVEHDR));
	// close the playback device
	waveInClose(hWaveIn);
	return;
}







int AudioRecorder::initializePlayback() {
	int		rc;

	// set up the format structure, needed for playback (and recording)
	setupFormat();

	// open the playback device
	rc = waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveFormat, (DWORD)NULL, 0, CALLBACK_NULL);
	if (rc) {
		cerr << "Unable to open Output sound Device! Error " << rc << endl;
		return(-1);
	}

	return(0);
}

int AudioRecorder::playBuffer() {
	static	WAVEFORMATEX WaveFormat;	/* WAVEFORMATEX structure for reading in the WAVE fmt chunk */
	static  WAVEHDR	WaveHeader;			/* WAVEHDR structure for this buffer */
	MMRESULT	mmErr;
	int		rc;

	// stop previous note (just in case)
	waveOutReset(hWaveOut);   // is this good?

	// get the header ready for playback
	waveHeaderOut.lpData = (char*)recBuf;
	waveHeaderOut.dwBufferLength = recBufSize * sizeof(short);
	rc = waveOutPrepareHeader(hWaveOut, &waveHeaderOut, sizeof(WAVEHDR));
	if (rc) {
		printf("Failed preparing WAVEHDR, error 0x%x.", rc);
		return(0);
	}
	waveHeaderOut.dwFlags &= ~(WHDR_BEGINLOOP | WHDR_ENDLOOP);

	// play the buffer. This is NON-blocking.
	mmErr = waveOutWrite(hWaveOut, &waveHeaderOut, sizeof(WAVEHDR));
	// wait for completion
	rc = waitOnHeader(&waveHeaderOut, 0);
	// give back resources
	waveOutUnprepareHeader(hWaveOut, &waveHeaderOut, sizeof(WAVEHDR));
	return(rc);
}

void AudioRecorder::closePlayback() {
	//int		i;
	//for (i = 0; i < NFREQUENCIES; ++i) {
	//	waveOutUnprepareHeader(hWaveOut, &WaveHeader[i], sizeof(WAVEHDR));
	//}

	waveOutUnprepareHeader(hWaveOut, &waveHeaderOut, sizeof(WAVEHDR));

	// close the playback device
	waveOutClose(hWaveOut);
	return;
}









int AudioRecorder::waitOnHeader(WAVEHDR* wh, char cDit) {
	long	lTime = 0;
	// wait for whatever is being played, to finish. Quit after 10 seconds.
	for (; ; ) {
		if (wh->dwFlags & WHDR_DONE) return(0);
		// idle for a bit so as to free CPU
		Sleep(100L);
		lTime += 100;
		if (lTime >= 30000) {
			cerr << "waitOnHeader(...) timed out!" << endl;
			return(-1);  // timeout period
		}
		if (cDit) printf("%c", cDit);
	}
}

