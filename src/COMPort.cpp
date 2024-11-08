#include "COMPort.h"
#include <iostream>
#include <cstring>


using namespace std;


#ifdef _WIN32
/*****************************************************************************
 * 							WINDOWS IMPLEMENTATION							 *
 *****************************************************************************/

// TODO: !!! put Windows-specific implementation here !!! 

#elif __linux__
/*****************************************************************************
 * 							LINUX IMPLEMENTATION							 *
 *****************************************************************************/

/*	RS232 Communication : Linux function implementations 
 *  Copyright 2019 Michael Galle 
 *  Edited by Matvey Regentov
 *  
 *  Source 1: https://www.cmrr.umn.edu/~strupp/serial.html
 * 	Source 2: http://www.tldp.org/HOWTO/Serial-Programming-HOWTO/index.html
 *  Source 3: http://man7.org/linux/man-pages/man3/termios.3.html 
 *  Source 4: https://man7.org/linux/manpages/man2/fcntl.2.html
 * 
 */

#include <stdio.h>	
#include <unistd.h>		// Unix standard functions
#include <fcntl.h>		// File control funcitons
#include <errno.h>		// Error number definitions
#include <termios.h>	// POSIX terminal control definitions
#endif

/*****************************************************************************
 * 							COMMON IMPLEMENTATION							 *
 *****************************************************************************/


COMPort::COMPort() {}

COMPort::COMPort(COMPortBaud baud, CPParity parity, int stop_bits) {
    this->baud = baud;
    this->parity = parity;
    this->stop_bits = stop_bits;
}

COMPort::~COMPort() {
    if (is_port_open) {
        closePort();
    }

    if (port_name != nullptr) {
        free(port_name);
    }
}

CPErrorCode COMPort::openPort(char* port_name) {
    if (is_port_open) {
        cerr << "Unable to open " << port_name 
             << ". Port is already open for " << this->port_name << endl;
        return CPErrorCode::PORT_IS_OPEN;
    }

    if (this->port_name != nullptr) {
        free(this->port_name);
    }
    this->port_name = (char*)malloc(strlen(port_name) * sizeof(char));
    strcpy(this->port_name, port_name);


    // TODO: !!! put code to open port here !!!


#if defined(__linux__)
    fd = open(port_name, O_RDWR | O_NOCTTY | O_NDELAY);	// Open for reading and writing, not as controlling terminal, no delay (no sleep - keep awake) (see Source 1)
	if (fd == -1) {
		perror("\nError: could not open specified port\n"); 
        return CPErrorCode::PARAMETER_ERROR;
	}

    fcntl(fd, F_SETFL, 0);						// Manipulates the file descriptor ands sets status FLAGS to 0 (see Source 4)
                                                // Block (wait) until characters come in or interval timer expires

    configPort();

#endif


    
    is_port_open = true;
    return CPErrorCode::SUCCESS;
}

CPErrorCode COMPort::sendMessage(void* buf, unsigned int num_bytes) {
    if (!is_port_open) {
        return CPErrorCode::PORT_IS_CLOSED;
    }

    writeToPort(buf, num_bytes);

    return CPErrorCode::SUCCESS;
}

CPErrorCode COMPort::receiveMessage(void* buf,
                                          size_t bufSize,
                                          size_t maxMessage,
                                          long int timeout_ms) {
    
    if (!is_port_open) {
        return CPErrorCode::PORT_IS_CLOSED;
    }

    readFromPort(buf, bufSize);

    return CPErrorCode::SUCCESS;
}

CPErrorCode COMPort::closePort() {
    if (!is_port_open) {
        return CPErrorCode::PORT_IS_CLOSED;
    }

    // TODO: !!! put code to close port !!!

#ifdef __linux__ 
    close(fd);
#endif

    return CPErrorCode::SUCCESS;
}



#ifdef _WIN32
/*****************************************************************************
 * 							WINDOWS IMPLEMENTATION							 *
 *****************************************************************************/

// TODO: !!! put Windows-specific implementation here !!! 

#elif __linux__
/*****************************************************************************
 * 							LINUX IMPLEMENTATION							 *
 *****************************************************************************/
void COMPort::setParity(termios* options) {
    switch (parity) {
    case CPParity::EVEN:
        options->c_cflag |= PARENB;		// enable parity
        options->c_cflag &= ~PARODD;    // unset -> even parity 
        break;
    case CPParity::ODD:
        options->c_cflag |= PARENB;		// enable parity
        options->c_cflag |= PARENB;		// set bit -> odd parity
        break;
    case CPParity::NONE:
        options->c_cflag &= ~PARENB;	// PARITY NOT ENABLED
        break;
    default:
        cerr << "Invalid parity setting: " << parity << endl;
        break;
    }
}

void COMPort::setNumStopBits(termios *options) {
    // CSTOPB = 2 stop bits (1 otherwise). Therefore ~CSTOPB means 1 stop bit
    switch (stop_bits) {
    case 1:
        options->c_cflag &= ~CSTOPB;
        break;
    case 2:
    	options->c_cflag |= CSTOPB;
        break;
    default:
        cerr << "Invalid number of stop bits: " << stop_bits << endl;
        break;
    }
}


CPErrorCode COMPort::configPort() {
    struct termios options;
	tcgetattr(fd, &options);						// Fills the termios structure with the current serial port configuration 

	// Change the current settings to new values

	// Set baud rate
	cfsetispeed(&options, baud);				// Input speed (rate)  -- Most systems don't support different input and output speeds so keep these the same for portability
	cfsetospeed(&options, baud);				// Output speed (rate)

	// Enable the receiver and set as local mode - CLOCAL & CREAD should always be enabled. 
	// CLOCAL so that program does not become owner of the port
	// CREAD so that the serial interface will read incoming data
	options.c_cflag |= (CLOCAL | CREAD);

	// Set other options to match settings on Windows side of comm (No parity, 1 stop bit) - See Source 1 
    setParity(&options);
    setNumStopBits(&options);
	options.c_cflag &= ~CSIZE;						// Mask the character size to be in bits
	options.c_cflag |= CS8;							// Use 8 bit data (per character)
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Set the port for RAW input with no echo or other input signals
	options.c_oflag &= ~OPOST;						// Use RAW output mode
	options.c_cc[VMIN] = 0;							// Min # of chars to read. When 0 VTIME specifies the wait time for every character
	options.c_cc[VTIME] = 240;						// Time in 1/10ths of a second to wait for every character before timing out. 
													// If VTIME is set to 0 then reads will block forever (port will only read)
	// Apply the new options to the port
	tcsetattr(fd, TCSANOW, &options); 				// TCSANOW makes changes without waiting for data to complete


    return CPErrorCode::SUCCESS;
}


CPErrorCode COMPort::writeToPort(void* buf, unsigned int num_bytes) {
    int written = write(fd, buf, num_bytes);
	if (written < 0) {
		cerr << "write() of bytes failed!\n" << endl;
        return CPErrorCode::WRITE_FAILED;
	}

    if (written != num_bytes) {
        cerr << "Warning! sent " << written << " bytes, but expected to send "
             << num_bytes << endl;
    }
    cout << written << endl;

    return CPErrorCode::SUCCESS;
}

CPErrorCode COMPort::readFromPort(void* buf, size_t bufSize) {
    int recBytes = read(fd, buf, bufSize);
    if (recBytes < 0) {
        cerr << "Failed to read message from comPort" << endl;
        return CPErrorCode::READ_FAILED;
    }

    cout << recBytes << endl;

    if (recBytes == 0) {
        cerr << "Warning! Reading EOF" << endl;
    }

    return CPErrorCode();
}

#endif