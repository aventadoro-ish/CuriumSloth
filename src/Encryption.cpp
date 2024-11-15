#include "Encryption.h"
#include <cstring>	
#include <iostream>

using namespace std;

void* XORencrypt(void* toEncrypt, size_t len, char c) {
    char key = c; //Any char will work
    void* output = (void*)malloc(len);
    memcpy(output, toEncrypt, len);
    
    
    for (int i = 0; i < len; i++){
        ((char*)output)[i] = ((char*)toEncrypt)[i] ^ key;
    }

    return output;
}

string XORdecrypt(string toDecrypt, char a) {
    char key = a; //Any char will work
    string output = toDecrypt;

    for (int i = 0; i < toDecrypt.size(); i++)
        output[i] = toDecrypt[i] ^ key;

    return output;
}