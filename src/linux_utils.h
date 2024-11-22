#pragma once

#ifdef __linux__
int kbhit(void);

#endif

void printHexDump(void* buf, size_t size);