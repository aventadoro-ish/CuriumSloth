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

	int initializeRecording();
	int recordBuffer();

public:

	int recordAudio(uint32_t seconds, uint32_t samplesPerSecond);
};

