/* This module provides implementations for compression algorithms as 
 * standalone functions (no class)
 * RLE - added by Besart Kalezic
 */

#pragma once

#include <iostream>
#include <cstdlib>

// Function Prototypes
long long RLEcompress(unsigned char* in, long long iInLen, unsigned char* out, long long iOutMax);
long long RLEdecompress(unsigned char* in, long long iInLen, unsigned char* out, long long iOutMax, unsigned char cEscape);
