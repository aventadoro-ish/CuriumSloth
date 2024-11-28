#include "COMPort.h"
#include <iostream>
#include <cstring>
#include <cmath>



// defines timeout in terms of a number of bytes at a certain baud
// if the time passes equivalent to the transmission of this number of bytes
// this is considered a timeout
#define TIMEOUT_IN_BYTES 2000

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
#include <fcntl.h>		// File control funcitons and dynamic blocking/non-blocking switch
#include <errno.h>		// Error number definitions
#include <termios.h>	// POSIX terminal control definitions
#include <sys/ioctl.h>

#include "utils.h"
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
    if (isPortOpen()) {
        closePort();
    }

    if (port_name != nullptr) {
        free(port_name);
    }
}

CPErrorCode COMPort::openPort(char* port_name) {
    if (isPortOpen()) {
        cerr << "Unable to open " << port_name 
             << ". Port is already open for " << this->port_name << endl;
        return CPErrorCode::PORT_IS_OPEN;
    }

    if (this->port_name != nullptr) {
        free(this->port_name);
    }
    this->port_name = (char*)malloc(strlen(port_name) * sizeof(char) + 1);
    strcpy(this->port_name, port_name);

    
    return openPortPlatform();;
}

CPErrorCode COMPort::sendMessage(void* buf, unsigned int num_bytes) {
    if (!isPortOpen()) {
        return CPErrorCode::PORT_IS_CLOSED;
    }

    // estimate the transmission time
    // 8 bits + parity if enabled + start bit + stop bits
    int num_bits_per_byte = 8 + abs((int)parity) + 1 + stop_bits;
    double transmission_time = num_bits_per_byte * num_bytes * 3.0 / (int)baud ;
    transmission_time += getTimeoutMs() / 1000.0;
    last_transmission_end = chrono::steady_clock::now() +
                            chrono::duration_cast<chrono::steady_clock::duration>(
                                    chrono::duration<double>(transmission_time));
    cout << "Transmission time = " << transmission_time << endl;


    return writeToPort(buf, num_bytes);;
}

CPErrorCode COMPort::receiveMessage(void* buf,
                                          size_t bufSize,
                                          size_t maxMessage) {
    
    if (!isPortOpen()) {
        return CPErrorCode::PORT_IS_CLOSED;
    }
    
    return readFromPort(buf, bufSize);
}

CPErrorCode COMPort::closePort() {
    if (!isPortOpen()) {
        return CPErrorCode::PORT_IS_CLOSED;
    }

    // TODO: !!! put code to close port !!!
    if (!CloseHandle(hCom)) {
        cerr << "Error closing COM port: " << GetLastError() << endl;
        return CPErrorCode::PARAMETER_ERROR;
    }

#ifdef __linux__ 
    close(fd);
#endif

    return CPErrorCode::SUCCESS;
}

unsigned int COMPort::getTimeoutMs() {
    int num_bits_per_byte = 8 + abs((int)parity) + 1 + stop_bits;
    double timeout = num_bits_per_byte * TIMEOUT_IN_BYTES * 1000 / (unsigned int)baud;
    return ceil(timeout);
}


#ifdef _WIN32
/*****************************************************************************
 * 							WINDOWS IMPLEMENTATION							 *
 *****************************************************************************/

CPErrorCode COMPort::writeToPort(void* buf, unsigned int num_bytes) {
    DWORD bytesWritten = 0;
    if (!WriteFile(hCom, buf, num_bytes, &bytesWritten, nullptr)) {
        cerr << "Error writing to COM port: " << GetLastError() << endl;
        return CPErrorCode::WRITE_FAILED;
    }
    
    if (bytesWritten != num_bytes) {
        cerr << "WARNING! written bytes " << bytesWritten 
        << " != requested to write " << num_bytes << endl;
    }
    
    return CPErrorCode::SUCCESS;
}

CPErrorCode COMPort::readFromPort(void* buf, size_t bufSize) {
    DWORD bytesRead = 0;
    if (!ReadFile(hCom, buf, bufSize, &bytesRead, nullptr)) {
        cerr << "Error reading from COM port: " << GetLastError() << endl;
        return CPErrorCode::READ_FAILED;
    }

    return CPErrorCode::SUCCESS;
}

CPErrorCode COMPort::openPortPlatform() {
    
    hCom = CreateFileA(
        port_name,
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
    );

    if (hCom == INVALID_HANDLE_VALUE) {
        cerr << "Error opening port: " << GetLastError() << endl;
        return CPErrorCode::PARAMETER_ERROR;
    }

    DCB dcb = { 0 };
    dcb.DCBlength = sizeof(DCB);

    if (!GetCommState(hCom, &dcb)) {
        cerr << "Error getting COM state: " << GetLastError() << endl;
        return CPErrorCode::PARAMETER_ERROR;
    }

    dcb.BaudRate = (DWORD)baud;
    dcb.ByteSize = 8; // Assuming 8 bits
    dcb.Parity = (parity == CPParity::EVEN) ? EVENPARITY :
        (parity == CPParity::ODD) ? ODDPARITY : NOPARITY;
    dcb.StopBits = (stop_bits == 2) ? TWOSTOPBITS : ONESTOPBIT;

    if (!SetCommState(hCom, &dcb)) {
        cerr << "Error setting COM state: " << GetLastError() << endl;
        return CPErrorCode::PARAMETER_ERROR;
    }

    // Setup timeouts
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;

    if (!SetCommTimeouts(hCom, &timeouts)) {
        cerr << "Error setting timeouts: " << GetLastError() << endl;
        return CPErrorCode::PARAMETER_ERROR;
    }

    return CPErrorCode::SUCCESS;
}

CPErrorCode COMPort::setNonBlockingMode() {
    cerr << "NOT IMPLEMENTED" << endl;
    return CPErrorCode::PARAMETER_ERROR;
}

CPErrorCode COMPort::setBlockingMode() {
    cerr << "NOT IMPLEMENTED" << endl;
    return CPErrorCode::PARAMETER_ERROR;
}

unsigned int COMPort::numInputBytes() {
    if (!isPortOpen()) {
        cerr << "Error: Port is not open." << endl;
        return 0;
    }

    // Check the COM port for input bytes
    COMSTAT comStat;
    DWORD errors;
    if (!ClearCommError(hCom, &errors, &comStat)) {
        cerr << "Error getting input buffer size: " << GetLastError() << endl;
        return 0;
    }
    return comStat.cbInQue; // Number of bytes in the input buffer
}

unsigned int COMPort::numOutputBytes() {
    if (!isPortOpen()) {
        cerr << "Error: Port is not open." << endl;
        return 0;
    }

    // Check the COM port for output bytes
    COMSTAT comStat;
    DWORD errors;
    if (!ClearCommError(hCom, &errors, &comStat)) {
        cerr << "Error getting output buffer size: " << GetLastError() << endl;
        return 0;
    }
    return comStat.cbOutQue; // Number of bytes in the output buffer
}

bool COMPort::isPortOpen() {
	return (hCom != INVALID_HANDLE_VALUE); // Check if the port is open
}

bool COMPort::canWrite() {
    auto now = std::chrono::steady_clock::now();

    if (now < last_transmission_end) {
        return false; // Still waiting for timeout to pass
    }

    // Calculate the elapsed time since the last transmission ended
    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_transmission_end);

    // Check if the elapsed time exceeds or matches the timeout
    if (elapsedTime.count() >= getTimeoutMs()) {
        return true;
    }
    else {
        return false;
    }
}


unsigned int getTimeoutMs();


#elif __linux__
/*****************************************************************************
 * 							LINUX IMPLEMENTATION							 *
 *****************************************************************************/
CPErrorCode COMPort::openPortPlatform() {
    fd = open(port_name, O_RDWR | O_NOCTTY); // | O_NDELAY);	// Open for reading and writing, not as controlling terminal, no delay (no sleep - keep awake) (see Source 1)
	if (fd == -1) {
		perror("\nError: could not open specified port\n"); 
        return CPErrorCode::PARAMETER_ERROR;
	}

    fcntl(fd, F_SETFL, 0);						// Manipulates the file descriptor ands sets status FLAGS to 0 (see Source 4)
                                                // Block (wait) until characters come in or interval timer expires

    CPErrorCode err = configPort();
    tcflush(fd, TCIFLUSH);
    return err;
}


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
        cerr << "Invalid parity setting: " << (int)parity << endl;
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
	cfsetispeed(&options, (speed_t)baud);				// Input speed (rate)  -- Most systems don't support different input and output speeds so keep these the same for portability
	cfsetospeed(&options, (speed_t)baud);				// Output speed (rate)

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
    
    // options.c_cc[VTIME] = 240;						// Time in 1/10ths of a second to wait for every character before timing out. 
    //                                                  // If VTIME is set to 0 then reads will block forever (port will only read)
    
    // number of 1/10ths of a second for a timeout to occur based on TIMEOUT_IN_BYTES
    float timeout = 80 * TIMEOUT_IN_BYTES / (int) baud;
    timeout = 0.9; // TODO: fix
    if (timeout < 1.0) {
        options.c_cc[VTIME] = 1;
        cout << "configPort() delay was clamped to 1" << endl;
    } else {
        options.c_cc[VTIME] = ceil(timeout);
        cout << "configPort() delay was set to " << ceil(timeout) << endl;
    }

    
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
        return CPErrorCode::WRITE_FAILED;
    }

    cout << "COM Port wrote " << hex << written << " bytes (attempted " << hex << num_bytes << " bytes)" << endl;
    return CPErrorCode::SUCCESS;
}

CPErrorCode COMPort::readFromPort(void* buf, size_t bufSize) {
    size_t bytesReceived = 0;
    
    while (bytesReceived < bufSize) {
        int recBytes = read(fd, buf + bytesReceived, bufSize - bytesReceived);

        if (recBytes < 0) {     // error
            cerr << "Failed to read message from comPort" << endl;
            return CPErrorCode::READ_FAILED;
        } else if (recBytes == 0) { // timeout
            break;
        }

        bytesReceived += recBytes;
    }

    if (bytesReceived > 0) {
        cout << "COM Port read " << hex << bytesReceived  << " bytes" << endl;
        return CPErrorCode::SUCCESS;
    } else {
        // nothing was received
        return CPErrorCode::READ_FAILED;
    }

}


CPErrorCode COMPort::setNonBlockingMode() {
    if (!isPortOpen()) {
        return CPErrorCode::PORT_IS_CLOSED;
    }

    // Enable non-blocking mode
    int flags = fcntl(fd, F_GETFL, 0);
    flags |= O_NONBLOCK;  // Set non-blocking flag
    fcntl(fd, F_SETFL, flags);
    return CPErrorCode();
}


CPErrorCode COMPort::setBlockingMode() {
    if (!isPortOpen()) {
        return CPErrorCode::PORT_IS_CLOSED;
    }
    

    // Enable blocking mode
    int flags = fcntl(fd, F_GETFL, 0);
    flags &= ~O_NONBLOCK;  // Clear non-blocking flag
    fcntl(fd, F_SETFL, flags);

}



unsigned int COMPort::numInputBytes() {
    int bytes_available = 0;
    ioctl(fd, FIONREAD, &bytes_available);
    
    if (bytes_available > 0) {
        return bytes_available;
    } else {
        return 0;
    }

}

unsigned int COMPort::numOutputBytes() {
    int bytes_in_output_queue = 0;
    ioctl(fd, TIOCOUTQ, &bytes_in_output_queue);

    if (bytes_in_output_queue > 0) {
        return bytes_in_output_queue;
    } else {
        return 0;
    }

    return 0;
}


bool COMPort::isPortOpen() {
    struct termios tty;

    if (tcgetattr(fd, &tty) == 0) {
        // Port is open and valid
        return true;
    } else {
        // Not a valid open port
        return false;
    }
    
}


bool COMPort::canWrite() {
    chrono::steady_clock::time_point now = chrono::steady_clock::now();
    
    if (now < last_transmission_end) {
        return false;
    }

    // Calculate elapsed time since the last transmission ended
    auto elapsed_time = chrono::duration_cast<chrono::milliseconds>
        (now - last_transmission_end);

    // Check if the elapsed time exceeds or matches the timeout
    if (elapsed_time.count() >= getTimeoutMs()) {
        // cout << "CAAAAAAAn WRITE" << endl;
        return true;
    } else {
        // cout << "CANT WRITE" << endl;
        return false;
    }
}



#endif