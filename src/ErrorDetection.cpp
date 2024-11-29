#include "ErrorDetection.h"

int ErrorDetection::computeChecksum(const std::string& payload) {
    int checksum = 0;
    for (char c : payload) {
        checksum += c;
    }
    return checksum % 256; // Modulo for 1-byte checksum
}

bool ErrorDetection::verifyChecksum(const std::string& payload, int checksum) {
    return computeChecksum(payload) == checksum;
}