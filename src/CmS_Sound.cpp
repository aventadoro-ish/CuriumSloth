#include "CmS_Sound.h"



#ifdef _WIN32
#include <mmsystem.h>					
#pragma comment(lib, "Ws2_32.lib")	   // Make sure we are linking against the Ws2_32.lib library
#pragma comment(lib, "Winmm.lib")      // Make sure we are linking against the Winmm.lib library - some functions/symbols from this library (Windows sound API) are used

#elif __linux__
#include <alsa/asoundlib.h>		// At terminal need to install alsa headers to /usr/include/alsa via installing: $ sudo apt-get install libasound2-dev   

// Constants (depend on hardware capabilities)
// #define SAMPLE_TIME         6       * 4   // 6 seconds. Total sample/playback time in seconds 
// #define SAMPLE_RATE         16384   * 4   // # of frames (samples) per second - using 65536/4 = 16384 gives 1 buffer/s
#define BYTES_PER_HW_BUF    65536         // 2^16 (based on hardware buffer max size)
// #define NUM_CHANNELS        2             // # of channels (choose 1 for mono, 2 for stereo)
// #define BYTES_PER_SAMPLE    2             // 16 Bit samples chosen = 2 bytes per sample (frame)
#define PERIODS_PER_HW_BUF  4  

#else
#error "Platform not supported"

#endif


#include <math.h>
#include <iostream>


#include <cstdlib>
#include <stdlib.h>

using namespace std;




AudioRecorder::AudioRecorder(
	uint32_t samplesPerSecond,
	uint16_t bitsPerSample,
	uint16_t numChannels) {

	recSamplesPerSencod = samplesPerSecond;
	recBitsPerSample = bitsPerSample;
	recNumCh = numChannels;

}

AudioRecorder::~AudioRecorder() {
	clearBuffer();
}

int AudioRecorder::recordAudio(uint32_t seconds) {
	prepareBuffer(seconds);

	initializeRecording();
	recordBuffer();
	closeRecordring();
	return 0;
}

int AudioRecorder::replayAudio() {
	if (recBuf == nullptr) {
		cerr << "ERROR! Unable to replay recBuf (nullptr)" << endl;
		return -1;
	}

	cout << " replayAudio()" << endl;

	initializePlayback();
	playBuffer();
	closePlayback();
	return 0;
}

void AudioRecorder::clearBuffer() {
	if (recBuf != nullptr) {
		free(recBuf);
		recBuf = nullptr;
		recBufSize = 0;
	}
}

int AudioRecorder::prepareBuffer(int seconds) {
	if (recBuf != nullptr) {
		cerr << "ERROR! Trying to rewrite recBuf!" << endl;
		return -1;
	}

#ifdef _WIN32
	long unsigned int bytesPerSample = recBitsPerSample / 8;
	long unsigned int bytesPerSecond = bytesPerSample * recNumCh * recSamplesPerSencod;
	long unsigned int bytesToAllocate = bytesPerSecond * seconds;
	// long unsigned int elementsToAllocate = bytesToAllocate / sizeof(short);
	// recBufSize = elementsToAllocate;
	recBufSize = bytesToAllocate;

	
	// recBufSize = seconds * recSamplesPerSencod * (recBitsPerSample / 8) * recNumCh;
	
	recBuf = (AudioBufT*)malloc(recBufSize);
	
#elif __linux__
	long unsigned int bytesPerSample = recBitsPerSample / 8;
	long unsigned int bytesPerSecond = bytesPerSample * recNumCh * recSamplesPerSencod;
	long unsigned int bytesToAllocate = bytesPerSecond * seconds;
	long unsigned int elementsToAllocate = bytesToAllocate / sizeof(char);
	recBufSize = elementsToAllocate;

	// Nuber of bytes in the buffer used to store the complete playback/capture data
	//			  5       * 2    	 * 16				/ 8 * 16384				   * 2
	// recBufSize = (seconds * recNumCh * recBitsPerSample / 8 * recSamplesPerSencod) * sizeof(AudioBufT);
	recBuf = (AudioBufT*)malloc(recBufSize);
	cout << "Allocated " << recBufSize << " bytes" << endl;
#else

#endif
	if (recBuf == nullptr) {
		cerr << "ERROR! Could not allocate memory for recBuf!" << endl;
		return -1;
	}

	return 0;
}

#if defined(_WIN32)
/*****************************************************************************
 * 							WINDOWS IMPLEMENTATION							 *
 *****************************************************************************/

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

int AudioRecorder::waitOnHeader(WAVEHDR* wh, char cDit) {
	long	lTime = 0;
	// wait for whatever is being played, to finish. Quit after 10 seconds.
	for (; ; ) {
		if (wh->dwFlags & WHDR_DONE) return(0);
		// idle for a bit so as to free CPU
		Sleep(100L);
		lTime += 100;
		if (lTime >= MAX_RECORDING_LEN) {
			cerr << "waitOnHeader(...) timed out!" << endl;
			return(-1);  // timeout period
		}
		if (cDit) cout << cDit;
	}
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
	ZeroMemory(&waveHeaderIn, sizeof(WAVEHDR)); // Ensure all fields are zeroed
	waveHeaderIn.lpData = (char*)recBuf;
	// waveHeaderIn.dwBufferLength = recBufSize * sizeof(short) - 1;
	waveHeaderIn.dwBufferLength = recBufSize;
	rc = waveInPrepareHeader(hWaveIn, &waveHeaderIn, sizeof(WAVEHDR));
	if (rc) {
		cerr << "Failed preparing input WAVEHDR, error " << rc << endl;
		return(-1);
	}

	return(0);
}

int AudioRecorder::recordBuffer() {
	//static  WAVEHDR	WaveHeader;			/* WAVEHDR structure for this buffer */
	MMRESULT	mmErr;
	int		rc;

	// stop previous recording (just in case)
	waveInReset(hWaveIn);   // is this good?


	waveHeaderIn.dwFlags &= ~(WHDR_BEGINLOOP | WHDR_ENDLOOP);

	// Give the buffer to the recording device to fill.
	mmErr = waveInAddBuffer(hWaveIn, &waveHeaderIn, sizeof(WAVEHDR));
	// Record the buffer. This is NON-blocking.
	mmErr = waveInStart(hWaveIn);

	// wait for completion
	rc = waitOnHeader(&waveHeaderIn);
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

	if (!recBuf || recBufSize == 0) {
		cerr << "ERROR: Playback buffer is invalid or empty." << endl;
		return -1;
	}

	// stop previous note (just in case)
	waveOutReset(hWaveOut);   // is this good?
	ZeroMemory(&waveHeaderOut, sizeof(WAVEHDR)); // Ensure all fields are zeroed


	// get the header ready for playback
	waveHeaderOut.lpData = (char*)recBuf;
	waveHeaderOut.dwBufferLength = recBufSize;                                    //*sizeof(short);
	mmErr = waveOutPrepareHeader(hWaveOut, &waveHeaderOut, sizeof(WAVEHDR));
	if (mmErr != MMSYSERR_NOERROR) {
		cout << "Failed preparing WAVEHDR, error " << mmErr << endl;
		return -1;
	}
	waveHeaderOut.dwFlags &= ~(WHDR_BEGINLOOP | WHDR_ENDLOOP);

	// play the buffer. This is NON-blocking.
	mmErr = waveOutWrite(hWaveOut, &waveHeaderOut, sizeof(WAVEHDR));
	if (mmErr != MMSYSERR_NOERROR) {
        cerr << "ERROR: waveOutWrite failed. Error: " << mmErr << endl;
        waveOutUnprepareHeader(hWaveOut, &waveHeaderOut, sizeof(WAVEHDR));
        return -1;
    }

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




#elif defined(__linux__)
/*****************************************************************************
 * 							LINUX IMPLEMENTATION							 *
 *****************************************************************************/

int AudioRecorder::initializeRecording() {
	int i = prepareStream(&pcm_handle_Capture, stream_Capture, &hwparams_Capture, pcm_name_Capture);
	return i;
}

int AudioRecorder::recordBuffer() {
	int BYTES_PER_FRAME = (recNumCh * recBitsPerSample / 8);      		// = 4 for 16 bit stereo
	int FRAMES_IN_BIG_BUF = recBufSize / BYTES_PER_FRAME;
	size_t framesR = 0;  
	size_t max = 0; 
	
	readbuf(pcm_handle_Capture, (char *)recBuf, FRAMES_IN_BIG_BUF, &framesR, &max);
	// readbuf(pcm_handle_Capture, (char *)recBuf, recBufSize, &framesR, &max);
	
	cout << "recordBuffer() recorded " << framesR << "frames" << endl;
	return 0;
}
void AudioRecorder::closeRecordring() {
	snd_pcm_close(pcm_handle_Capture);
}

int AudioRecorder::initializePlayback() {
	cout << "init playback" << endl;
	int i = prepareStream(&pcm_handle_Playback, stream_Playback, &hwparams_Playback, pcm_name_Playback);
	cout << "init playback: " << i << endl;
	return i;
}
int AudioRecorder::playBuffer() {

	int BYTES_PER_FRAME = (recNumCh * recBitsPerSample / 8);      		// = 4 for 16 bit stereo
	int FRAMES_IN_BIG_BUF = recBufSize / BYTES_PER_FRAME;
	size_t framesW = 0;  
	writebuf(pcm_handle_Playback, (AudioBufT*)recBuf, FRAMES_IN_BIG_BUF, &framesW);
	// writebuf(pcm_handle_Playback, (AudioBufT*)recBuf, recBufSize, &framesW);
	cout << "playBuffer() replayed " << framesW << "frames" << endl;

	return 0;
}
void AudioRecorder::closePlayback() {
	snd_pcm_close(pcm_handle_Playback);
}


int AudioRecorder::prepareStream(snd_pcm_t** pcm_handle, snd_pcm_stream_t stream, snd_pcm_hw_params_t** hwparams, char* pcm_name) {
	// Variables
	unsigned int exact_rate;			// Sample rate near requested that the system is capable of: using snd_pcm_hw_params_set_rate_near()
	snd_pcm_uframes_t exact_frames;		// If the buffer (max size is 16384 frames [16384 * 4 bytes]) cannot support the number of frames desired in 'frames', a function below changes this value to 'exact_frames' supported
	int dir;							// if exact_rate == rate then dir = 0, if exact_rate < rate then dir = -1, if exact_rate > rate then dir = 1

	int BYTES_PER_FRAME = (recNumCh * recBitsPerSample / 8);      		// = 4 for 16 bit stereo, = 2 for 16 bit mono
	int FRAMES_PER_HW_BUF  = (BYTES_PER_HW_BUF / BYTES_PER_FRAME);    	// Number of frames in a completely full hardware buffer
	
	cout << "AudioRecorder::prepareStream() buf is @" << (void*)recBuf << " contains " << recBufSize << " bytes" << endl;
	
	// Allocate space for the hwparams structure on the stack
	snd_pcm_hw_params_alloca(hwparams);

	// Open the PCM device (for playback)
	if(snd_pcm_open(pcm_handle, pcm_name, stream, stream) < 0) {
		fprintf(stderr, "Error opening PCM device %s\n", pcm_name);
		return(-1); // Fail
	}

	// Specify the control parameters of the PCM device 
	// (access type, buffer size, # of channels, sample format, sample rate, number of periods, etc)
	// First read the values of whats currently there into 'hwparams' and then change what we want
	if (snd_pcm_hw_params_any(*pcm_handle, *hwparams) < 0) {
		fprintf(stderr, "Can not get configuration this PCM device. \n"); 
		return(-1); // Fail
	}

	// Set the access type to Read/Write Interleaved Data (alternating words of sample data for the left and right channel)
	if (snd_pcm_hw_params_set_access(*pcm_handle, *hwparams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0){
		fprintf(stderr, "Error setting access type\n");
		return(-1);
	}

	// Set sample format to 16 bit little endian
	if (snd_pcm_hw_params_set_format(*pcm_handle, *hwparams, SND_PCM_FORMAT_S16_LE) < 0){
		fprintf(stderr, "Error setting format\n");
		return(-1);
	}

	// Set sample rate.
	// If the exact rate is not supported by the hardware, use the nearest rate
	exact_rate = recSamplesPerSencod; 
	if (snd_pcm_hw_params_set_rate_near(*pcm_handle, *hwparams, &exact_rate, &dir) < 0){
		fprintf(stderr, "Error setting rate\n");
		return(-1);
	}
	if(dir != 0) {
		fprintf(stderr, "The rate %d Hz is not supported by your hardware. Using %d Hz instead.\n", recSamplesPerSencod, exact_rate);
	}

	// Set number of channels (2 for stereo or 1 for mono)
	if(snd_pcm_hw_params_set_channels(*pcm_handle, *hwparams, recNumCh) < 0) {
		fprintf(stderr, "Error setting channels.\n");
		return(-1);
	}

	// Set number of periods per hardware buffer (a.k.a. 'fragments')
	if(snd_pcm_hw_params_set_periods(*pcm_handle, *hwparams, (unsigned int)PERIODS_PER_HW_BUF, 0) < 0 ) {
		fprintf(stderr, "Error setting periods\n");
		return(-1);
	}

	// Set hardware buffer size in terms of a number of frames
	// A 'frame' is the number of bytes in all channels per sample (e.g. 16 bit has 2 bytes per channel * 2 channels = 4 bytes per frame)
	exact_frames = FRAMES_PER_HW_BUF;
	if (snd_pcm_hw_params_set_buffer_size_near(*pcm_handle, *hwparams, &exact_frames) < 0) {
		fprintf(stderr, "Error setting buffer size. \n");
		return(-1);
	}
	if(exact_frames != (snd_pcm_uframes_t)FRAMES_PER_HW_BUF) {
		fprintf(stderr, "The buffer size of %lu frames is not supported by your hardware. Using buffer size of %lu frames instead.\n", (snd_pcm_uframes_t)FRAMES_PER_HW_BUF, exact_frames);
	}

	// Apply the configuration properties to the PCM device pointed to by pcm_handle
	if(snd_pcm_hw_params(*pcm_handle, *hwparams) < 0) {
		fprintf(stderr, "Error setting HW params.\n");
		return(-1);
	}

	//printf("End of prepare stream function\n");				// Test success
	return(0);  // Success
}

// record audio
long AudioRecorder::readbuf(snd_pcm_t *handle, char *buf, long len, size_t *frames, size_t *max) {
    long r;
	_snd_pcm_format format = SND_PCM_FORMAT_S16_LE;
	int frame_bytes = (snd_pcm_format_width(format) / 8) * recNumCh; 
    
	while (len > 0) {
        r = snd_pcm_readi(handle, buf, len);		
        if (r == -EAGAIN)
		    continue;
		if (r < 0)
            return r;
        // printf("read = %li\n", r);
        buf += r * frame_bytes;
        len -= r;
        *frames += r;
		if ((long)*max < r)
            *max = r;
    }
    return r;
}

long AudioRecorder::writebuf(snd_pcm_t *handle, char *buf, long len, size_t *frames) {
    long r;
	_snd_pcm_format format = SND_PCM_FORMAT_S16_LE;
    int frame_bytes = (snd_pcm_format_width(format) / 8) * recNumCh;
    while (len > 0) {
        r = snd_pcm_writei(handle, buf, len);		
        if (r == -EAGAIN)
            continue;
        //printf("write = %li\n", r);
        if (r < 0)
            return r;
        buf += r * frame_bytes;
        len -= r;
        *frames += r;
    }
    return 0;
}


#else

#endif
