#pragma once

#include <cstddef>
#include <chrono>

#ifdef _WIN32
// TODO: !!! put Windows-specific stuff here !!! 
#include <Windows.h>
#include <string>
#elif __linux__
#include <termios.h>	// POSIX terminal control definitions
// #include <sys/ioctl.h>

#else
#error "Platform not supported"

#endif

enum class  CPParity {
    EVEN = 1,
    ODD = -1,
    NONE = 0
};

enum class COMPortBaud {
    COM_BAUD_4800       = 4800,
    COM_BAUD_9600       = 9600,
    COM_BAUD_19200      = 19200,
    COM_BAUD_38400      = 38400,
    COM_BAUD_57600      = 57600,
    COM_BAUD_115200     = 115200,
    COM_BAUD_230400     = 230400,
    COM_BAUD_460800     = 460800,
    COM_BAUD_660800     = 650000, // 660800,   // non-standard
    COM_BAUD_MAX        = 750000,
    COM_BAUD_921600     = 921600
};

enum class CPErrorCode {
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
    COMPortBaud baud = COMPortBaud::COM_BAUD_9600;
    CPParity parity = CPParity::NONE;
    int stop_bits = 1;
    bool is_port_open; 

    std::chrono::steady_clock::time_point last_transmission_end;


#ifdef _WIN32
    // TODO: !!! put Windows-specific member variables here !!! 
    HANDLE hCom; // Windows handle for the COM port
    COMMTIMEOUTS timeouts; // Timeout settings
#elif __linux__

    int fd; // file descriptor for the port

#endif



#ifdef _WIN32
    CPErrorCode writeToPort(void* buf, unsigned int num_bytes);
    
    CPErrorCode readFromPort(void* buf, size_t bufSize, size_t* bytesRead = nullptr);

    CPErrorCode openPortPlatform();

#elif __linux__
    CPErrorCode configPort();

    void setParity(termios* options);
    void setNumStopBits(termios* options);

    CPErrorCode writeToPort(void* buf, unsigned int num_bytes);
    
    CPErrorCode readFromPort(void* buf, size_t bufSize);
    CPErrorCode openPortPlatform();


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
    /// @param readBytes pass a pointer to size_t to get the number of bytes read
    /// @param maxMessage (NOT IMPLEMENTED) stop receiving after maxMessage number of bytes are received. Set to 0 to default to bufSize
    /// @return 0 if success
    CPErrorCode receiveMessage(
        void* buf, 
        std::size_t bufSize,
        std::size_t* readBytes = nullptr,
        std::size_t maxMessage = 0);


    CPErrorCode closePort();
    
    
    /// @brief DOES NOT WORK ATM TODO: fix
    /// @return 
    CPErrorCode setNonBlockingMode();

    /// @brief DOES NOT WORK ATM TODO: fix
    /// @return 
    CPErrorCode setBlockingMode();

    /// @brief returns the number of bytes in the hardware input buffer that 
    /// are pending to be read
    /// @return number of bytes
    unsigned int numInputBytes();

    /// @brief returns the number of bytes in the hardware output buffer that
    /// are pending to be sent
    /// @return number of bytes
    unsigned int numOutputBytes();

    /// @brief checks if the port port_name is a valid port, and it is
    /// currently open
    /// @return true if port is open
    bool isPortOpen();

    /// @brief Ensures a timeout delay between sendMessage() calls
    /// @return true if timeout has passed since last call, false otherwise
    bool canWrite();


    /// @brief Returns the timout time in ms based on TIMEOUT_IN_BYTES 
    /// and baud rate
    /// @return number of ms
    unsigned int getTimeoutMs();


    unsigned int getTimeToTransmitNBytesMs(size_t n_bytes);

};
