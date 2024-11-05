#include "CmS_Sound.h"



#ifdef _WIN32
#include <mmsystem.h>					
#pragma comment(lib, "Ws2_32.lib")	   // Make sure we are linking against the Ws2_32.lib library
#pragma comment(lib, "Winmm.lib")      // Make sure we are linking against the Winmm.lib library - some functions/symbols from this library (Windows sound API) are used
#elif __linux__
#include <alsa/asoundlib.h>		// At terminal need to install alsa headers to /usr/include/alsa via installing: $ sudo apt-get install libasound2-dev   
#include "stream.h"


#else
#error "Platform not supported"
#endif


#include <math.h>

#include <iostream>

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

	recBufSize = seconds * recSamplesPerSencod;
	recBuf = (short*)malloc(recBufSize * sizeof(short));
	if (recBuf == nullptr) {
		cerr << "ERROR! Could not allocate memory for recBuf!" << endl;
		return -1;
	}

	return 0;
}

/*****************************************************************************
 * 							WINDOWS IMPLEMENTATION							 *
 *****************************************************************************/
#if defined(_WIN32)

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




#elif defined(__linux__)
/*****************************************************************************
 * 							LINUX IMPLEMENTATION							 *
 *****************************************************************************/

int AudioRecorder::initializeRecording() {
	return -1;
}
int AudioRecorder::recordBuffer() {
	return -1;
}
void AudioRecorder::closeRecordring() {
}

int AudioRecorder::initializePlayback() {
	return -1;
}
int AudioRecorder::playBuffer() {
	return -1;
}
void AudioRecorder::closePlayback() {
}


int prepareStream(snd_pcm_t** pcm_handle, snd_pcm_stream_t stream, snd_pcm_hw_params_t** hwparams, char* pcm_name) {
		// Variables
        unsigned int exact_rate;			// Sample rate near requested that the system is capable of: using snd_pcm_hw_params_set_rate_near()
        snd_pcm_uframes_t exact_frames;		// If the buffer (max size is 16384 frames [16384 * 4 bytes]) cannot support the number of frames desired in 'frames', a function below changes this value to 'exact_frames' supported
        int dir;							// if exact_rate == rate then dir = 0, if exact_rate < rate then dir = -1, if exact_rate > rate then dir = 1

		int BYTES_PER_FRAME = (NUM_CHANNELS * BYTES_PER_SAMPLE);      		// = 4 for 16 bit stereo, = 2 for 16 bit mono
		int FRAMES_PER_HW_BUF  = (BYTES_PER_HW_BUF / BYTES_PER_FRAME);    	// Number of frames in a completely full hardware buffer
		
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
	    exact_rate = SAMPLE_RATE; 
	    if (snd_pcm_hw_params_set_rate_near(*pcm_handle, *hwparams, &exact_rate, &dir) < 0){
	    	fprintf(stderr, "Error setting rate\n");
	    	return(-1);
	    }
	    if(dir != 0) {
	    	fprintf(stderr, "The rate %d Hz is not supported by your hardware. Using %d Hz instead.\n", SAMPLE_RATE, exact_rate);
	    }
	
	    // Set number of channels (2 for stereo or 1 for mono)
	    if(snd_pcm_hw_params_set_channels(*pcm_handle, *hwparams, NUM_CHANNELS) < 0) {
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

int demo() {
		// Variables *****
	// Based on constants
	int BYTES_PER_FRAME = (NUM_CHANNELS * BYTES_PER_SAMPLE);      		// = 4 for 16 bit stereo
	int BYTES_IN_BIG_BUF = (SAMPLE_TIME * BYTES_PER_HW_BUF);       		// Nuber of bytes in the buffer used to store the complete playback/capture data
	int FRAMES_IN_BIG_BUF = BYTES_IN_BIG_BUF / BYTES_PER_FRAME;

	// Playback vars
	snd_pcm_t* pcm_handle_Playback; 									// Handle for a PCM device (playback)
	snd_pcm_stream_t stream_Playback = SND_PCM_STREAM_PLAYBACK; 		// *** Playback stream ***
	snd_pcm_hw_params_t* hwparams_Playback;								// Structure used to specify config properties of hardware/stream
	char pcm_name_Playback[] = "hw:0,0"; //"hw:0,1"; //"default";		// Name of PCM device (e.g. "hw:0,0" or "default"). First # is number of soundcard, second # is the device. Use $ aplay -L  or $arecord -L to get names of available devices 
	
	// Capture (Record) vars
	snd_pcm_t* pcm_handle_Capture; 										// Handle for a PCM device (capture)
	snd_pcm_stream_t stream_Capture = SND_PCM_STREAM_CAPTURE; 			// *** Capture stream ***
	snd_pcm_hw_params_t* hwparams_Capture;								// Structure used to specify config properties of hardware/stream
	char pcm_name_Capture[] = "hw:0,0"; //"default";					// Name of PCM device (e.g. "hw:0,0" or "default"). First # is number of soundcard, second # is the device. Use $ aplay -L  or $arecord -L to get names of available devices 

	// Common vars
	int i;																// Error checking and counting
	char* bigBuf;														// Large buffer to store entire recording
	char* tempBuf;														// Pointer to sections in bigBuf. It advances by the size of the hw buffer every loop 
	bigBuf = (char*)malloc(BYTES_IN_BIG_BUF); 							// Buffer to store catured data 

	// Prepare Capture (record) stream
	i = prepareStream(&pcm_handle_Capture, stream_Capture, &hwparams_Capture, pcm_name_Capture);
	printf("Testing: Result of prepare capture stream process: %s\n", (i == 0) ? "ok": "failed");
	printf("Testing:Capture PCM name: %s\n", pcm_name_Capture);
	

	// Prepare Playback stream
	printf("\n\nNow recording ...\n\n");
	i = prepareStream(&pcm_handle_Playback, stream_Playback, &hwparams_Playback, pcm_name_Playback);
	printf("Testing: Result of prepare playback stream process: %s\n", (i == 0) ? "ok": "failed");
	printf("Testing: Playback PCM name: %s\n", pcm_name_Playback);
	

	// Start capture (recording) PCM data from device (mic) using the interleaved data function readi()
	tempBuf = bigBuf;									// Temporary pointer that advances by size of hw buffer for every loop 
	size_t framesR = 0;
	size_t framesW = 0;  
	size_t max = 0; 

	
	readbuf(pcm_handle_Capture, (char *)tempBuf, FRAMES_IN_BIG_BUF, &framesR, &max);
	printf("Testing: Frames read = %lu\n", framesR);
	
	printf("\n\nNow playing recording ...\n\n");
	writebuf(pcm_handle_Playback, (char*)bigBuf, FRAMES_IN_BIG_BUF, &framesW);
	printf("Testing: Frames written = %lu\n", framesW);

		
	// Need to close handles
	snd_pcm_close(pcm_handle_Capture);
	snd_pcm_close(pcm_handle_Playback);

	return(0); 
}

long readbuf(snd_pcm_t *handle, char *buf, long len, size_t *frames, size_t *max)
{
    long r;
	_snd_pcm_format format = SND_PCM_FORMAT_S16_LE;
	int channels = 2;
	int frame_bytes = (snd_pcm_format_width(format) / 8) * channels; 
    
	while (len > 0) {
        r = snd_pcm_readi(handle, buf, len);		
        if (r == -EAGAIN)
		    continue;
		if (r < 0)
            return r;
        //printf("read = %li\n", r);
        buf += r * frame_bytes;
        len -= r;
        *frames += r;
		if ((long)*max < r)
            *max = r;
    }
    return r;
}

long writebuf(snd_pcm_t *handle, char *buf, long len, size_t *frames)
{
    long r;
	int channels = 2;
	_snd_pcm_format format = SND_PCM_FORMAT_S16_LE;
    int frame_bytes = (snd_pcm_format_width(format) / 8) * channels;
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
