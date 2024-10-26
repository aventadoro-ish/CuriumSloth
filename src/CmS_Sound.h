#pragma once

#ifdef _WIN32
#include <windows.h>					// Contains WAVEFORMATEX structure
#elif __linux__

#else
#error "Platform not supported"
#endif



#include <cstdint>

#define	NFREQUENCIES		96		// number of frequencies used - leave this alone
#define MAX_RECORDING_LEN	30000	// timeout for recording and replay (ms)

class AudioRecorder {
private:

#ifdef _WIN32
	WAVEFORMATEX	waveFormat;			/* WAVEFORMATEX structure for reading in the WAVE fmt chunk */
	
	// recording variables
	HWAVEIN		hWaveIn;				/* Handle of opened WAVE In device */
	WAVEHDR		waveHeaderIn;			/* Header of WAVE In */

	// replay variables
	HWAVEOUT	hWaveOut;				/* Handle of opened WAVE Out device */
	WAVEHDR		waveHeaderOut;			/* Header of WAVE Out */
	//WAVEHDR		waveHeader[NFREQUENCIES];	/* WAVEHDR structures - 1 per buffer */
#elif __linux__

#else

#endif

	uint32_t recSamplesPerSencod = 8000;
	uint16_t recBitsPerSample = 16;
	uint16_t recNumCh = 1;

	short* recBuf = nullptr;
	uint32_t recBufSize = 0;



#ifdef _WIN32
	/// <summary>
	/// Sets up WAVEFORMATEX format of waveFormat member
	/// </summary>
	void setupFormat();
	
	/// <summary>
	/// Wait for completion of recroding/replay of wh (with MAX_RECORDING_LEN timeout)
	/// </summary>
	/// <param name="wh">audio header to wait for</param>
	/// <param name="cDit">char to put to console while waiting (0 for not printing)</param>
	/// <returns> 0 if success, -1 if timeout</returns>
	int waitOnHeader(WAVEHDR* wh, char cDit = 0);
#elif __linux__

#else

#endif

	/// <summary>
	/// Allocates memory for recBuf. Checks as to not overwrite the buffer
	/// </summary>
	/// <param name="seconds">number of seconds to prepare the buffer for</param>
	/// <returns>0 if success</returns>
	int prepareBuffer(int seconds);


	/* ------------- Recording functions -------------- */

	/// <summary>
	/// Prepares member variables for a new recording
	/// </summary>
	/// <returns> 0 if success </returns>
	int initializeRecording();
	
	/// <summary>
	/// Records audio to a member buffer using member parameters
	/// </summary>
	/// <returns> 0 if success </returns>
	int recordBuffer();

	/// <summary>
	/// Cleans up after a recording completes
	/// </summary>
	void closeRecordring();

	/* ------------- Replay functions ----------------- */

	/// <summary>
	/// Prepares member variables for audio replay
	/// </summary>
	/// <returns>0 if success</returns>
	int initializePlayback();

	/// <summary>
	/// Replays the audio from recBuf
	/// </summary>
	/// <returns>0 if success</returns>
	int playBuffer();

	/// <summary>
	/// Cleans up after a replay finishes
	/// </summary>
	void closePlayback();

public:

	AudioRecorder(
		uint32_t samplesPerSecond = 8000,
		uint16_t bitsPerSample = 16,
		uint16_t numChannels = 1);
	~AudioRecorder();

	/// <summary>
	/// Record audio of given length. Cannot overwrite the member buffer
	/// </summary>
	/// <param name="seconds">number of seconds to recrod for</param>
	/// <returns>0 if success</returns>
	int recordAudio(uint32_t seconds);

	/// <summary>
	/// Replays the audio from the buffer. 
	/// Gets audio length based on the buffer size and waveFormat parameters 
	/// given as constructor parameters
	/// </summary>
	/// <returns></returns>
	int replayAudio();

	/// <summary>
	/// Frees up buffer memory
	/// </summary>
	void clearBuffer();

	/// <summary>
	/// A way to get the pointer to the recBuf
	/// </summary>
	/// <returns>recBuf pointer (may be nullptr)</returns>
	short* getBuffer() {
		return recBuf;
	}

	/// <summary>
	/// Get buffer size
	/// </summary>
	/// <returns>buffer size (0 for empty buffer)</returns>
	uint32_t getBufferSize() {
		return recBufSize;
	}

	/// <summary>
	/// Allows to pass a prerecorded buffer for replay. 
	/// Depends on the waveFormat to be compatible with the buffer
	/// </summary>
	/// <param name="buf">Buffer to be replayed</param>
	/// <param name="bufSize">Size of the buffer</param>
	void setBuffer(short* buf, uint32_t bufSize) {
		recBuf = buf;
		recBufSize = bufSize;
	}

};


int	soundTest();

