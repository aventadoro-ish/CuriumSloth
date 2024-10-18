#pragma once

#include <windows.h>					// Contains WAVEFORMATEX structure
#include <cstdint>

#define	NFREQUENCIES		96		// number of frequencies used - leave this alone


class AudioRecorder {
private:
	WAVEFORMATEX	waveFormat;			/* WAVEFORMATEX structure for reading in the WAVE fmt chunk */
	HWAVEOUT		hWaveOut;				/* Handle of opened WAVE Out and In device */
	HWAVEIN			hWaveIn;
	WAVEHDR			waveHeader[NFREQUENCIES];	/* WAVEHDR structures - 1 per buffer */
	WAVEHDR			waveHeaderSilence;
	WAVEHDR			waveHeaderIn;


	uint32_t recLength = 1;
	uint32_t recSamplesPerSencod = 8000;
	short* recBuf = nullptr;
	uint32_t recBufSize = 0;

	/// <summary>
	/// Sets up WAVEFORMATEX format of waveFormat member using parameters
	/// </summary>
	/// <param name="numChannels">number of channels for recording. Default: 1</param>
	/// <param name="samplesPerSecond">number of samples per second. Default: 8000</param>
	/// <param name="bitsPerSample">number of bits per sample. Default: 8</param>
	void setupFormat(
		uint32_t numChannels = 1,
		uint32_t samplesPerSecond = 8000,
		uint16_t bitsPerSample = 8);

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
	/// Wait for completion of recroding/replay of wh (with 10sec timeout)
	/// </summary>
	/// <param name="wh">audio header to wait for</param>
	/// <param name="cDit">char to put to console while waiting</param>
	/// <returns> 0 if success, -1 if timeout</returns>
	int waitOnHeader(WAVEHDR* wh, char cDit);




public:

	/// <summary>
	/// Record audio with given parameters. Cannot overwrite the member buffer
	/// </summary>
	/// <param name="seconds">number of seconds to recrod for</param>
	/// <param name="samplesPerSecond">samples per second to record</param>
	/// <returns>0 if success</returns>
	int recordAudio(uint32_t seconds, uint32_t samplesPerSecond);
};

