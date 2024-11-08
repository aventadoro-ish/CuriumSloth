#include "COMPort.h"
#include <iostream>
#include <cstring>


using namespace std;

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
    
    is_port_open = true;
    return CPErrorCode::SUCCESS;
}

CPErrorCode COMPort::sendMessage(void* buf, unsigned int num_bytes) {
    if (!is_port_open) {
        return CPErrorCode::PORT_IS_CLOSED;
    }

    // TODO: !!! put code to send a message here !!!

    return CPErrorCode::SUCCESS;
}

CPErrorCode COMPort::receiveMessage(void* buf,
                                          size_t bufSize,
                                          size_t maxMessage,
                                          long int timeout_ms) {
    
    if (!is_port_open) {
        return CPErrorCode::PORT_IS_CLOSED;
    }


    // TODO: !!! put code to receive a message here !!!

    return CPErrorCode::SUCCESS;
}

CPErrorCode COMPort::closePort() {
    if (!is_port_open) {
        return CPErrorCode::PORT_IS_CLOSED;
    }

    // TODO: !!! put code to close port !!!

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

// TODO: !!! put Windows-specific implementation here !!! 

#endif