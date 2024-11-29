#ifndef ERROR_CORRECTION_H
#define ERROR_CORRECTION_H
#include <vector>

class HammingCode {
public:
    // Encodes the data using Hamming code
    std::vector<int> encode(const std::vector<int>& data);

    // Decodes the encoded data and corrects any single-bit errors
    std::vector<int> decode(const std::vector<int>& encodedData);

private:
    // Helper method to calculate the parity bit value for a given position
    int getParityBitValue(const std::vector<int>& encodedData, int parityPos);

    // Helper method to calculate the error position using syndrome
    int getErrorPosition(const std::vector<int>& encodedData);

    // Corrects the error by flipping the bit at the error position
    void correctError(std::vector<int>& encodedData, int errorPosition);
};

#endif