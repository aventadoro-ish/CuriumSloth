#include "Compression.h"

#include <Windows.h>    
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

using namespace std;

long long RLEcompress(unsigned char* in, long long iInLen, unsigned char* out, long long iOutMax) {
	long long i = 0;	 
    long long outPos = 0; 
    unsigned char cEsc = 0x1b; 
    char count; 
    unsigned char repeatedChar;

	while (i < iInLen) {
		count = 0;
		do {	// Count the number of repeats until repeats stop (or until 255 characters reached)
			memcpy(&repeatedChar, &in[i], 1);		// Store the repeated character using memcpy()
			count++;// Increment the repeat counter

            // TODO: ++i works in VSC, but not in VS, i++ works in VS but not VSC
		} while ((memcmp(&in[i], &in[++i], 1) == 0) && (count <= 255) && (i < iInLen)); // memcmp() == 0 when same

		// handle escape as a special case
		if (repeatedChar == cEsc) {
			if (count <= 2) {
				// single ESC gets encoded as [ESC, 0]
				// double ESC gets encoded as [ESC, 1]
				out[outPos++] = cEsc; 
				out[outPos++] = count - 1;	// 0 for a single ESC, 1 for double ESC
			} else {
				// more than 2 ESC chars repeated
				out[outPos++] = cEsc; 
				out[outPos++] = 2;
				out[outPos++] = count;
			}

			continue; // skip the part that processes normal chars
		} 


		// RLE encode (compress only if there are 3 or more repeats)
		if (count >= 3) {
			memcpy(&out[outPos++], &cEsc, 1); // 3 or more repeats
			memcpy(&out[outPos++], &count, 1);// Here, integers (HEX values not strings) are used rather than strings
			memcpy(&out[outPos++], &repeatedChar, 1);
		}
		else if (count == 2) {
			// Copy the repeated character twice into the output buffer
			for (int i = 0; i < count; i++) {
				memcpy(&out[outPos++], &repeatedChar, 1); //callingthe repeated char twice
			}
		}		      // Your choice
		else {
			memcpy(&out[outPos++], &repeatedChar, 1);
		}

	}
	return(outPos);	   		// Stores the length of the output buffer � make sure it is less than iOutMax
}

long long RLEdecompress(unsigned char* in, long long iInLen, unsigned char* out, long long iOutMax, unsigned char cEsc) {
	long long i = 0; long long outPos = 0; char count; unsigned char repeatedChar; int j;
	while (i < iInLen) {
		// if this char is ESC				and this char and next char are not the same (true if this char is ESC, next char isn't ESC)
		if ((memcmp(&in[i], &cEsc, 1) == 0) && (memcmp(&in[i], &in[i + 1], 1) != 0)) {   // No repeats of ESE char, only one

			// handle special case for [ESC, 0], [ESC, 1], [ESC, 2, count]
			if (in[i+1] == 0) { // single ESC char
				out[outPos++] = cEsc;
				i += 2;	// processed this ESC and 0 (2 bytes)
			} else if (in[i+1] == 1) {	// double ESC char
				out[outPos++] = cEsc;
				out[outPos++] = cEsc;
				i += 2;
			} else if (in[i+1] == 2) {	// run of ESC
				count = in[i+2];	
				for (j = 0; j < count; j++) {
					memcpy(&out[outPos++], &cEsc, 1);
				}
				i += 3;					// Increment i by 3 (the RLE encoding length)
			} else {// handle normal chars
			
				memcpy(&count, &in[i + 1], 1);		// Next byte stores repeat count
				memcpy(&repeatedChar, &in[i + 2], 1);	// Next byte stores repeated character
				i += 3;				// Increment i by 3 (the RLE encoding length)
				for (j = 0; j < count; j++) {
					memcpy(&out[outPos++], &repeatedChar, 1);// Output repeated character to output buffer one character at a time
				}

			}




			// cout << i << " " << outPos << " : " ;
			// for (int iii = 0; iii < outPos; iii++) {
			// 	cout << hex << (int)out[iii] << " ";
			// }
			// cout << endl;


		}
		else {				// Single character - no repeats
			repeatedChar = in[i++];
			memcpy(&out[outPos++], &repeatedChar, 1);
			// cout << "*" << i  << " " << outPos << endl;
		}
	}
	if (outPos <= iOutMax) {			// Output buffer is less than max buffer size
		return(outPos);
	}
	else {				// Output buffer exceeds max
		printf("Buffer overflow: Cannot decompress");
		return(0);
	}
}

