#include "utils.h"

#include <iostream>
#include <cstring>	
#include <iomanip>

#ifdef WIN32
#include <windows.h>
#elif _POSIX_C_SOURCE >= 199309L
#include <time.h>   // for nanosleep
#else
#include <unistd.h> // for usleep
#endif


void sleep_ms(unsigned int milliseconds) { // cross-platform sleep function
    /* Implementation taken from:
     * https://stackoverflow.com/a/28827188
     */

#ifdef WIN32
    Sleep(milliseconds);
#elif _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
#else
    if (milliseconds >= 1000) {
      sleep(milliseconds / 1000);
    }
    usleep((milliseconds % 1000) * 1000);
#endif
}


#ifdef __linux__

/* Code from:
 * https://cboard.cprogramming.com/c-programming/63166-kbhit-linux.html
 * by user: itsme86
 */
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }

  return 0;
}


#endif



void printHexDump(void* buf, size_t size) {
    unsigned char* data = static_cast<unsigned char*>(buf);

    for (size_t i = 0; i < size; i += 16) {
        // Print the offset in hex
        // std::cout << std::setw(8) << std::setfill('0') << std::hex << i << ": ";
        std::cout << std::setw(8) << std::setfill('0') << std::hex << buf << ": ";

        // Print hex bytes
        for (size_t j = 0; j < 16; ++j) {
            if (i + j < size) {
                std::cout << std::setw(2) << static_cast<unsigned>(data[i + j]) << " ";
            } else {
                std::cout << "   "; // Fill space for alignment
            }
        }

        // Print ASCII characters
        std::cout << " |";
        for (size_t j = 0; j < 16; ++j) {
            if (i + j < size) {
                char c = data[i + j];
                if (std::isprint(static_cast<unsigned char>(c))) {
                    std::cout << c;
                } else {
                    std::cout << '.';
                }
            }
        }
        std::cout << "|" << std::endl;
    }

    // Reset the formatting
    std::cout << std::dec;
}