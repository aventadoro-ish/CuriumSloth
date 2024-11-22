/* Implementation: Functions for File IO - Getting random messages from a file
*  By: Michael Galle
*/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>			// Dynamic memory allocation for arrays that store quote location and length
#include "message.h"

#include <iostream>
#define FILE_NAME			"FortuneCookies.txt"	// name of the file with quotes
#define SEPARATOR_CH		'%'						// '%" is a separator. SEPARATOR_COUNT of '%' in a row delimits the quotes
#define SEPARATOR_COUNT		2						// "%%" is a separator sequence

#define _CRT_SECURE_NO_WARNINGS

#define MIN(a, b) a < b ? a : b;


// Function gets a random number between min and max (max is the number of quotes in the file)
int frandNum(int min, int max) {
	return (rand() % (max - min)) + min;
}

// Function returns number of quotes in the file (only need to run once)
int fnumQuotes(void) {
	FILE* file;
	// safely open file in read mode, get error code
	errno_t err = fopen_s(&file, FILE_NAME, "r");

	if (err != 0) {
		std::cout << "Failed to open " FILE_NAME "! error: " << strerror(err) << std::endl;
		return -1;
	}

	int num_separators = 0;
	int num_sep_chars = 0;	// number of consecutive separator chars

	char ch;
	for (;;) {
		ch = fgetc(file);
		if (ch == EOF) { break; }	// end of file exits the loop

		if (ch == SEPARATOR_CH) { num_sep_chars++; }

		// skip if SEPARATOR_CH are not consecutive
		if (num_sep_chars > 0 && num_sep_chars < SEPARATOR_COUNT && ch != SEPARATOR_CH) {
			num_sep_chars = 0;
		}

		if (num_sep_chars == SEPARATOR_COUNT) { 
			num_sep_chars = 0;
			num_separators++;
		}

	}

	fclose(file);
	// separators come before and after each quote, so num quotes is 1 less than the number of separators
	return num_separators - 1;
}

// Function returns an array that indicates the start of every quote in the file (number of characters from the start of the file) 
long int* fquoteIndices(int numQuotes) {
	long int* quote_starts = (long int*)malloc(sizeof(long int) * numQuotes);
	
	if (quote_starts == nullptr) {
		std::cout << "ERROR! Unable to create quote start indices  array" << std::endl;
		return nullptr;
	}

	FILE* file;
	errno_t err = fopen_s(&file, FILE_NAME, "r");

	if (err != 0) {
		std::cout << "ERROR! Failed to open " FILE_NAME "! error: " << strerror(err) << std::endl;
		return nullptr;
	}

	long int char_idx = 0;	// current char to be processed
	int quote_num = 0;		// quote number
	int num_sep = 0;		// number of consecutive separator chars
	bool skip_nl = false;	// once the separator has been processed, skip new-line-related chars 
	char ch;

	for (;;) {
		ch = fgetc(file);

		// all the quotes have been processed
		if (quote_num == numQuotes) { break; }

		// exit the loop on end of file
		if (ch == EOF) { break; }

		// count separators
		if (ch == SEPARATOR_CH) { num_sep++; }

		// skip if SEPARATOR_CH are not consecutive
		if (num_sep > 0 && num_sep < SEPARATOR_COUNT && ch != SEPARATOR_CH) {
			num_sep = 0;
		}

		// SEPARATOR_COUNT of consecutive SEPARATOR_CH were encountered
		if (num_sep == SEPARATOR_COUNT) {
			char_idx++;

			// skip the new lines and carrige returns before the quote starts
			ch = fgetc(file);
			while (ch == '\r' || ch == '\n') {
				char_idx++;
				ch = fgetc(file);
			}

			char_idx++;
			// this is the actual start of the quote
			quote_starts[quote_num] = char_idx;
			quote_num++;
			num_sep = 0;

			// next iteration
			continue;
		}

		char_idx++;

	}

	fclose(file);

	return quote_starts;
}

// Function returns the smaller of the actual quote length or MAX_QUOTE_LENGTH
int* fquoteLength(int numQuotes, long int* quoteIndices) {
	// invalid input check
	if (quoteIndices == nullptr) { return nullptr; }
	if (numQuotes <= 0) { return nullptr; }

	int* quote_lengths = (int*)malloc(sizeof(int*) * numQuotes);
	if (quote_lengths == nullptr) { 
		std::cout << "ERROR! Unable to create quote lengths array" << std::endl;
		return nullptr;
	}

	// due to usage of both \r\n and \n in the file
	//   accurate length calculation requires reading the file again
	FILE* file;
	errno_t err = fopen_s(&file, FILE_NAME, "r");

	if (err != 0) {
		std::cout << "ERROR! Failed to open " FILE_NAME "! error: " << strerror(err) << std::endl;
		return nullptr;
	}


	char ch;
	long int ch_idx = 0;
	int quote_num = 0;


	for (int i = 0; i < numQuotes; i++) {
		// for each quote
		
		// skip to the starting position of the quote
		while (ch_idx < quoteIndices[i]) {
			ch = fgetc(file);
			ch_idx++;

			if (ch == EOF) {
				// erroneous premature end of file
				std::cout << "ERROR! Premature EOF encountered while computing quote lengths!" << std::endl;
				free(quote_lengths);
				return nullptr;
			}
		}


		// once the start of the quote is reached, count chars before the next separator
		int length = 0;
		long int start_idx = ch_idx;
		long int last_non_nl_char_idx = 0;
		int num_sep_chars = 0;
		for (;;) {
			ch = fgetc(file);
			ch_idx++;

			if (ch == EOF) {
				std::cout << "ERROR! Premature EOF encountered while computing quote lengths!" << std::endl;
				length = start_idx - ch_idx - 1;
				break;
			}

			if (ch != '\n' && ch != '\r' && ch != SEPARATOR_CH) {
				last_non_nl_char_idx = ch_idx;
			}

			if (ch == SEPARATOR_CH) {
				num_sep_chars++;
			}

			if (num_sep_chars == SEPARATOR_COUNT) {
				// end of quote found
				length = last_non_nl_char_idx - start_idx + 1;
				break;
			}


		}

		quote_lengths[i] = MIN(length, MAX_QUOTE_LENGTH - 1);

	}

	fclose(file);

	return quote_lengths;

} 

// Function that gets q random quote from the FortuneCookies file 
int GetMessageFromFile(
	char* buff,
	int iLen, 
	int randNum, 
	int numQuotes, 
	long int* quoteIndices, 
	int* quoteLengths
	) {

	if (buff == nullptr) { return -1; }
	if (randNum > numQuotes) { return -1; }


	FILE* file;
	errno_t err = fopen_s(&file, FILE_NAME, "r");

	if (err != 0) {
		std::cout << "Failed to open " FILE_NAME "! error: " << strerror(err) << std::endl;
		return -1;
	}

	// skip to the needed quote
	for (int i = 0; i < quoteIndices[randNum] - 1; i++) {
		// if end of file reached here, error occured
		if (fgetc(file) == EOF) {
			std::cout << "Reading a quote that is outside of the bounds of the file!" << std::endl;
			fclose(file);
			return -1;
		}
	}

	// copy the quote into the buffer
	for (int i = 0; i < quoteLengths[randNum]; i++) {
		char ch = fgetc(file);
		if (ch == EOF) {
			std::cout << "Reading a quote that is outside of the bounds of the file!" << std::endl;
			fclose(file);
			return -1;
		}
		buff[i] = ch;

	}
	buff[quoteLengths[randNum]] = 0; // add string terminator to the end

	fclose(file);

	return 0;
	

}
