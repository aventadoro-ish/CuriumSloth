#include "CmS_Sound.h"
#pragma comment(lib, "Ws2_32.lib")	   // Make sure we are linking against the Ws2_32.lib library
#pragma comment(lib, "Winmm.lib")      // Make sure we are linking against the Winmm.lib library - some functions/symbols from this library (Windows sound API) are used
#include <mmsystem.h>					
#include <math.h>

#include <iostream>

using namespace std;



AudioRecorder::AudioRecorder(uint32_t samplesPerSecond, uint16_t bitsPerSample) {
	recSamplesPerSencod = samplesPerSecond;
	recBitsPerSample = bitsPerSample;

	//cout << MMSYSERR_INVALHANDLE << " - 'MMSYSERR_INVALHANDLE'" << endl;
	//cout << MMSYSERR_NODRIVER << " - 'MMSYSERR_NODRIVER'" << endl;
	//cout << MMSYSERR_NOMEM << " - 'MMSYSERR_NOMEM'" << endl;

}

AudioRecorder::~AudioRecorder() {
}





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
	
	// set up the format structure, needed for recording.
	// setupFormat();
	// should be setup in Constructor


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
	waveHeaderIn.dwFlags &= ~(WHDR_BEGINLOOP | WHDR_ENDLOOP);

	return(0);
}

int AudioRecorder::recordBuffer() {
	MMRESULT	mmErr;
	int		rc;

	mmErr = waveInAddBuffer(hWaveIn, &waveHeaderIn, sizeof(WAVEHDR));
	if (mmErr) {
		cerr << "ERROR! waveInAddBuffer(...) returned " << mmErr << endl;
		return -1;
	}

	mmErr = waveInStart(hWaveIn);
	if (mmErr) {
		cerr << "ERROR! waveInStart(...) returned " << mmErr << endl;
		return -1;
	}

	rc = waitOnHeader(&waveHeaderIn, '.');
	if (rc) {
		cerr << "ERROR! waitOnHeader(...) returned " << rc << endl;
		return -1;
	}

	mmErr = waveInStop(hWaveIn);
	if (mmErr) {
		cerr << "ERROR! waveInStop(...) returned " << mmErr << endl;
		return -1;
	}

	return 0;




	// stop previous recording (just in case)
	waveInReset(hWaveIn);   // is this good?

	// done in initializeRecording()
	//waveHeaderIn.lpData = (char*)recBuf;
	//waveHeaderIn.dwBufferLength = recBufSize * sizeof(short);
	//rc = waveInPrepareHeader(hWaveIn, &waveHeaderIn, sizeof(WAVEHDR));
	//if (rc) {
	//	cout << "Failed preparing WAVEHDR, error: " << rc << endl;
	//	return(-1);
	//}
	//waveHeaderIn.dwFlags &= ~(WHDR_BEGINLOOP | WHDR_ENDLOOP);

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

int AudioRecorder::recordAudio(uint32_t seconds) {
	MMRESULT rc;

	setupFormat(1, recSamplesPerSencod, recBitsPerSample);

	// ***************************************
	// prepare recording buffer
	// ***************************************
	if (recBuf != nullptr) {
		cerr << "ERROR! Trying to record to a buffer that is occupied!" << endl;
		return -1;
	}

	// calculate the required buffer size
	recLength = seconds;
	recBufSize = recLength * recSamplesPerSencod * (recBitsPerSample / 8);

	// allocate space for the recording buffer
	recBuf = (short*)malloc(recBufSize);
	if (recBuf == nullptr) {
		cerr << "ERROR! Unable to allocate the recording buffer!" << endl;
	}

	// ***************************************
	// initialize recording (open device and prepare header)
	// ***************************************

	// open the recording device
	rc = waveInOpen(&hWaveIn, WAVE_MAPPER, &waveFormat, (DWORD)NULL, 0, CALLBACK_NULL);
	if (rc) {
		cerr << "Unable to open Input sound Device! Error: " << rc << endl;
		return(-1);
	}

	// prepare the buffer header 
	waveHeaderIn.lpData = (LPSTR)recBuf;
	waveHeaderIn.dwBufferLength = recBufSize * sizeof(short);
	//waveHeaderIn.dwFlags = 0;
	//waveHeaderIn.dwLoops = 0;

	rc = waveInPrepareHeader(hWaveIn, &waveHeaderIn, sizeof(WAVEHDR));
	if (rc) {
		cerr << "Failed preparing input WAVEHDR, error: " << rc << endl;
		return(-1);
	}
	// waveHeaderIn.dwFlags &= ~(WHDR_BEGINLOOP | WHDR_ENDLOOP);

	// ***************************************
	// record audio buffer
	// ***************************************
	int res = recordBuffer();
	if (res != 0) {
		cerr << "ERROR! recordBuffer() failed inside recordAudio(..)" << endl;
		return -1;
	}

	closeRecordring();

	return 0;
}








int AudioRecorder::replayAudio() {
	if (recBuf == nullptr) {
		cout << "ERROR! Trying to replay a null buffer!" << endl;
		return -1;
	}

	initializePlayback();

	int res = playBuffer();
	if (res != 0) {
		cout << "ERROR! playBuffer() failed inside replayAudio()" << endl;
		return -1;
	}

	return 0;
}



void AudioRecorder::closeRecordring() {
	waveInUnprepareHeader(hWaveIn, &waveHeaderIn, sizeof(WAVEHDR));
	// close the recording device
	waveInClose(hWaveIn);
	return;
}








int AudioRecorder::waitOnHeader(WAVEHDR* wh, char cDit) {
	long lTime = 0;
	// wait for whatever is being played, to finish. Quit after 10 seconds.
	for (;;) {
		if (wh->dwFlags & WHDR_DONE) {
			return(0);
		}
		
		// idle for a bit so as to free CPU
		Sleep(100L);
		lTime += 100;
		if (lTime >= 10000) {
			cout << "ERROR! waitOnHeader(...) timed out" << endl;
			return(-1);  // timeout period
	
		}
		if (cDit) printf("%c", cDit);
	}
}


int AudioRecorder::initializePlayback() {
	int		rc;
	// set up the format structure, needed for playback (and recording)
	setupFormat(1, recSamplesPerSencod);
	// open the playback device
	rc = waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveFormat, (DWORD)NULL, 0, CALLBACK_NULL);
	if (rc) {
		printf("Unable to open Output sound Device! Error %x.", rc);
		return(-1);
	}
	return(0);
}

int AudioRecorder::playBuffer() {
	MMRESULT	mmErr;
	int		rc;

	// stop previous note (just in case)
	waveOutReset(hWaveOut);   // is this good?

	rc = waveOutPrepareHeader(hWaveOut, &waveHeaderOut, sizeof(WAVEHDR));
	if (rc) {
		printf("Failed preparing WAVEHDR, error 0x%x.", rc);
		return(-1);
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
	for (int i = 0; i < NFREQUENCIES; ++i) {
		waveOutUnprepareHeader(hWaveOut, &waveHeader[i], sizeof(WAVEHDR));
	}
	// close the playback device
	waveOutClose(hWaveOut);
	return;
}
