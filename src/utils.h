/* This module defines cross-platform compatibility functions used throughout
 * the program as well as simple debugging tools
 * Author: Matvey Regentov
 */
#pragma once

#include <cstring>	// for size_t


void sleep_ms(unsigned int milliseconds); // cross-platform sleep function



#ifdef __linux__
int kbhit(void);

#endif

void printHexDump(void* buf, size_t size);