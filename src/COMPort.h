#pragma once

#include <cstddef>

#ifdef _WIN32
// TODO: !!! put Windows-specific stuff here !!! 
#elif __linux__
#include <termios.h>	// POSIX terminal control definitions


#else
#error "Platform not supported"

#endif

enum CPParity {
    EVEN,
    ODD,
    NONE
};

enum COMPortBaud {
    BAUD_4800       = 4800,
    BAUD_9600       = 9600,
    BAUD_19200      = 19200,
    BAUD_38400      = 38400,
    BAUD_57600      = 57600,
    BAUD_115200     = 115200,
    BAUD_230400     = 230400,
    BAUD_460800     = 460800,
    BAUD_921600     = 921600
};

enum CPErrorCode {
    SUCCESS,
    PORT_IS_CLOSED,
    PORT_IS_OPEN,
    MESSAGE_TOO_LONG,
    TIMED_OUT,
    WRITE_FAILED,
    READ_FAILED,
    PARAMETER_ERROR
};

class COMPort {
    char* port_name = nullptr;
    COMPortBaud baud = COMPortBaud::BAUD_9600;
    CPParity parity = CPParity::NONE;
    int stop_bits = 1;
    bool is_port_open = false;

#ifdef _WIN32
    // TODO: !!! put Windows-specific member variables here !!! 
#elif __linux__
    // TODO: !!! put Windows-specific member variables here !!! 
    int fd; // file descriptor for the port

#endif



#ifdef _WIN32
    // TODO: !!! put Windows-specific method declarations here !!! 

#elif __linux__
    // TODO: !!! put Windows-specific method declaration here !!! 
    CPErrorCode configPort();

    void setParity(termios* options);
    void setNumStopBits(termios* options);

    CPErrorCode writeToPort(void* buf, unsigned int num_bytes);
    
    CPErrorCode readFromPort(void* buf, size_t bufSize);

#endif

public:
    COMPort();
    COMPort(COMPortBaud baud, CPParity parity, int stop_bits);
    ~COMPort();

    /// @brief Tries to open a COM port based on the given name. 
    /// Allocates memory for a copy of port_name
    /// @param port_name COM port to open
    /// @return 0 if success
    CPErrorCode openPort(char* port_name);

    /// @brief Sends the provided array bytes as message 
    /// @param buf pointer to the message to be sent
    /// @param num_bytes number of bytes to send
    /// @return 0 if success
    CPErrorCode sendMessage(void* buf, unsigned int num_bytes);

    /// @brief Tries to receive the message from an open COM port
    /// @param buf pointer to the buffer for the message
    /// @param bufSize buffer size in bytes
    /// @param maxMessage stop receiving after maxMessage number of bytes are received. Set to 0 to default to bufSize
    /// @param timeout_ms stop receiving after this amount of time (ms). Set to 0 to disable
    /// @return 0 if success
    CPErrorCode receiveMessage(
        void* buf, 
        std::size_t bufSize,
        std::size_t maxMessage = 0,
        long int timeout_ms = 0);


    CPErrorCode closePort();

};