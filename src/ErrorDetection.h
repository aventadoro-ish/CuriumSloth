#ifndef ERRORDETECTION_H
#define ERRORDETECTION_H

#include <string>

class ErrorDetection {
public:
    // Compute checksum for a given payload
    static int computeChecksum(const std::string& payload);

    // Verify the checksum for a given payload
    static bool verifyChecksum(const std::string& payload, int checksum);

    // Enable or disable error detection
    static bool isErrorDetectionEnabled;
};

#endif