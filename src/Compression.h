/* This module provides implementations for compression algorithms as 
 * standalone functions (no class)
 * RLE - added by Besart Kalezic
 */

#pragma once

#include <iostream>
#include <cstdlib>
#include "adpcm.h"

// Function Prototypes
long long RLEcompress(unsigned char* in, long long iInLen, unsigned char* out, long long iOutMax);
long long RLEdecompress(unsigned char* in, long long iInLen, unsigned char* out, long long iOutMax, unsigned char cEscape);


// ADPCM Audio Compression
void ADPCMCompress(void* source, void* destination, const WAVEHeader &wav, ADPCMHeader &adpcm);

void ADPCMDecompress(void* source, void* destination, const ADPCMHeader &adpcm);
