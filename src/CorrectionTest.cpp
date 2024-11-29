#include <iostream>
#include "ErrorCorrection.h"
#include "ErrorDetection.h"
using namespace std;

int main() {
      
        vector<int> data = {
            1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
            1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
            1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
            1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
            1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0
        };
        cout << "This is the data\n";
        for (int act : data) cout << act;
        cout << endl;
        // Encoding the data
        HammingCode hamming;
        vector<int> encodedData = hamming.encode(data);

        cout << "Encoded Data: ";
        for (int bit : encodedData) {
            cout << bit;
        }
        cout << endl;

        // Simulate a bit error (flip one bit)
        encodedData[4] ^= 1;  // Error at position 4 (index 84)

        cout << "Corrupted Data: ";
        for (int bit : encodedData) {
            cout << bit;
        }
        cout << endl;

        // Decode the data
        vector<int> decodedData = hamming.decode(encodedData);

        cout << "Decoded Data: ";
        for (int bit : decodedData) {
            cout << bit;
        }
        cout <<endl;

        return 0;
    }
