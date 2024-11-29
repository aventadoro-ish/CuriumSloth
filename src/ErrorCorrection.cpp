#include "ErrorCorrection.h"
#include <iostream>
#include <stdexcept>
#include <iostream>
#include <cmath>

std::vector<int> HammingCode::encode(const std::vector<int>& data) {
    int dataSize = data.size();

    // Calculate the required number of parity bits (using the Hamming formula)
    int paritySize = 0;
    while ((1 << paritySize) < (dataSize + paritySize + 1)) {
        paritySize++;
    }

    int totalSize = dataSize + paritySize;
    std::vector<int> encodedData(totalSize, 0);  // Initialize with zeros

    // Insert the data into the encoded data, skipping parity bit positions
    int dataIndex = 0;
    for (int i = 0; i < totalSize; i++) {
        // Skip parity bit positions (1, 2, 4, 8, 16, etc.)
        if (((i + 1) & i) != 0) {  // Check if it's a parity bit position
            encodedData[i] = data[dataIndex++];
        }
    }

    // Calculate parity bits and insert them at the corresponding positions
    for (int i = 0; i < paritySize; i++) {
        int parityPos = (1 << i) - 1;  // Parity bit positions: 1, 2, 4, 8, etc.
        int parityValue = getParityBitValue(encodedData, parityPos);
        encodedData[parityPos] = parityValue;
    }

    return encodedData;
}

int HammingCode::getParityBitValue(const std::vector<int>& encodedData, int parityPos) {
    int parityValue = 0;
    for (int i = 0; i < encodedData.size(); i++) {
        // XOR the bits that correspond to the parity position
        if ((i + 1) & (parityPos + 1)) {
            parityValue ^= encodedData[i];
        }
    }
    return parityValue;
}

int HammingCode::getErrorPosition(const std::vector<int>& encodedData) {
    int errorPosition = 0;
    int size = encodedData.size();

    // Calculate the syndrome: XOR of positions covered by each parity bit
    for (int i = 0; i < size; i++) {
        if (encodedData[i] == 1) {
            errorPosition ^= (i + 1);
        }
    }
    return errorPosition;
}

void HammingCode::correctError(std::vector<int>& encodedData, int errorPosition) {
    if (errorPosition > 0 && errorPosition <= encodedData.size()) {
        encodedData[errorPosition - 1] ^= 1;  // Flip the corrupted bit
    }
}
std::vector<int> HammingCode::decode(const std::vector<int>& encodedData) {  // Keep const
    std::vector<int> modifiableData = encodedData;  // Create a copy to modify
    int errorPosition = getErrorPosition(modifiableData);
    if (errorPosition != 0) {
        correctError(modifiableData, errorPosition);  // Correcting the error
        std::cout << "Error detected at position: " << errorPosition << std::endl;
    }

    // Extract the original data from the corrected encoded data
    std::vector<int> decodedData;
    for (int i = 0; i < modifiableData.size(); i++) {
        if (((i + 1) & i) != 0) {  // Skip parity bit positions
            decodedData.push_back(modifiableData[i]);
        }
    }

    return decodedData;
}
