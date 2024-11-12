#include "Message.h"
#include <iostream>
#include <stdlib.h>	
#include <cstring>	
#include <iomanip>
#include <cstdint>

using namespace std;


void printHexDump(void* buf, size_t size);


Message::Message() {
  bufA = nullptr;
  sizeA = 0;

  bufB = nullptr;
  sizeB = 0;

  bufC = nullptr;
  sizeC = 0;

  bufO = nullptr;
  sizeO = 0;

  encryptionKey = 0;
  isEncode = true;

  header = MSGHeader();
}

Message::~Message() {
    if (bufA != nullptr) { free(bufA); }
    if (bufB != nullptr) { free(bufB); }
    if (bufC != nullptr) { free(bufC); }
    if (bufO != nullptr) { free(bufO); }

}




int Message::encryptMessage() {
    if (bufA == nullptr) {
        cerr << "ERROR! Can't encrypt an nullptr buffer" << endl;
        return -1;
    }

    if (bufB != nullptr) {
        cerr << "WARNING! Overriding bufB" << endl;
        return -1;
    }


    switch (header.encryption) {
    case MSGEncryption::NONE:
        bufB = (void*)malloc(sizeA);
        sizeB = sizeA;
        memcpy(bufB, bufA, sizeB);
        break;
    
    default:
        // TODO: Add more encryption standards support
        cerr << "WARNING! Encryption is not supported";
        return -1;
        break;
    }

    free(bufA);
    bufA = nullptr;
    sizeA = 0;

    return 0;
}

int Message::compressMessage() {
     if (bufB == nullptr) {
        cerr << "ERROR! Can't compress an nullptr buffer" << endl;
        return -1;
    }

    if (bufC != nullptr) {
        cerr << "WARNING! Overriding bufC" << endl;
        return -1;
    }


    switch (header.compression) {
    case MSGCompression::NONE:
        bufC = (void*)malloc(sizeB);
        sizeC = sizeB;
        memcpy(bufC, bufB, sizeC);
        break;
    
    default:
        // TODO: Add more compression standards support
        cerr << "WARNING! Compression is not supported";
        return -1;
        break;
    }

    free(bufB);
    bufB = nullptr;
    sizeB = 0;
    
    return 0;
}

int Message::calculateChecksum() {
    if (bufC == nullptr) {
        cerr << "ERROR! Can't compress an nullptr buffer" << endl;
        return -1;
    }

    unsigned int* bufInt = (unsigned int*)bufC;
    unsigned int sum = 0;
    for (int i = 0; i < sizeC / sizeof(unsigned int); i++) {
        sum += bufInt[i];
    }

    footer.checksum = sum;

    return 0;
}

int Message::prepareOutput() {
    if (bufC == nullptr) {
        cerr << "ERROR! Can't prepare an nullptr buffer" << endl;
        return -1;
    }

    if (bufO != nullptr) {
        cerr << "WARNING! Overriding bufO" << endl;
        return -1;
    }

    size_t total_size = sizeof(MSGHeader) + sizeC + sizeof(MSGFooter);
    // cout << "Output buffer size = " << total_size << endl;
    // cout << "Message size = " << strlen((char*)bufC) << endl;

    bufO = (void*)malloc(total_size);
    if (!bufO) {
        cerr << "ERROR! Unable to allocate memory for output buffer!" << endl;
        return -1;
    }
    sizeO = total_size;

    // cout << "bufO starts at " << hex << (unsigned long int)bufO << endl;
    // cout << "header starts at " << hex << (unsigned long int)bufO + sizeof(MSGHeader) << "\t" << sizeof(MSGHeader) << endl;
    // cout << "footer starts at " << hex << (unsigned long int)bufO + total_size - sizeof(MSGFooter) << "\t" << sizeof(MSGFooter)  << endl;

    void* payloadStartIdx = (void*)((unsigned long int)bufO + sizeof(MSGHeader));
    memcpy(payloadStartIdx, bufC, sizeC);

    memcpy(bufO, &header, sizeof(MSGHeader));
    

    void* footerStartIdx = (void*)((unsigned long int)bufO + total_size - sizeof(MSGFooter));
    memcpy(footerStartIdx, &footer, sizeof(MSGFooter));
    

    return 0;
}





int Message::getHeader() {
    if (bufA == nullptr) {
        cerr << "ERROR! Trying to read header from nullptr buffer" << endl;
        return -1;
    }

    memcpy(&header, bufA, sizeof(MSGHeader));

    printHeader();

    return 0;
}

int Message::decompressMessage() {
    if (bufA == nullptr) {
        cerr << "ERROR! Can't decompress an nullptr buffer" << endl;
        return -1;
    }

    if (bufB != nullptr) {
        cerr << "WARNING! Overriding bufB" << endl;
        return -1;
    }



    switch (header.compression) {
    case MSGCompression::NONE:
        sizeB = header.payloadSize;
        bufB = (void*)malloc(sizeB);
        memcpy(bufB, (void*)((unsigned int)bufA + sizeof(MSGHeader)), sizeB);
        break;
    
    default:
        // TODO: Add more decompression standards support
        cerr << "WARNING! Decompression is not supported" << endl;
        return -1;
        break;
    }

    // cout << "Decompressed message" << endl;
    // printHexDump(bufB, sizeB);

    free(bufA);
    bufA = nullptr;
    sizeA = 0;

    return 0;
}

int Message::decryptMessage() {
    if (bufB == nullptr) {
        cerr << "ERROR! Can't decrypt an nullptr buffer" << endl;
        return -1;
    }

    if (bufC != nullptr) {
        cerr << "WARNING! Overriding bufC" << endl;
        return -1;
    }


    switch (header.encryption) {
    case MSGEncryption::NONE:
        bufC = (void*)malloc(sizeB);
        sizeC = sizeB;
        memcpy(bufC, bufB, sizeC);
        break;
    
    default:
        // TODO: Add more decryption standards support
        cerr << "WARNING! Decryption is not supported";
        return -1;
        break;
    }

    free(bufB);
    bufB = nullptr;
    sizeB = 0;

    // cout << "Decrypted message" << endl;
    // printHexDump(bufC, sizeC);
    
    
    return 0;
}

int Message::preparePayload() {
    if (bufC == nullptr) {
        cerr << "ERROR! Can't prepare an nullptr buffer" << endl;
        return -1;
    }

    if (bufO != nullptr) {
        cerr << "WARNING! Overriding bufO" << endl;
        return -1;
    }

    bufO = (void*)malloc(sizeC);
    sizeO = sizeC;
    memcpy(bufO, bufC, sizeO);

    // cout << "Payload message" << endl;
    // printHexDump(bufO, sizeO);

    return 0;
}




int Message::addData(void* buf, size_t size, bool encode) {
    if (bufA != nullptr) {
        cerr << "ERROR! Trying to reallocate message data buffer!" << endl;
        return -1;
    }

    bufA = (void*)malloc(size);
    sizeA = size;
    if (!bufA) {
        cerr << "ERROR! Unable to allocate memory for data buffer." << endl;
        return -1;
    }

    memcpy(bufA, buf, size);
    isEncode = encode;

    return 0;
}

int Message::setEncryptionKey(unsigned int key) {
    encryptionKey = key;
    return 0;
}

void* Message::getMessage() {
    return bufO;
}

size_t Message::getMessageSize() {
    return sizeO;
}






void Message::printHeader() {
    cout << "Message header:" << endl;
    cout << "\tSender:      " << header.senderID << endl; 
    cout << "\tReceiver:    " << header.receiverID << endl; 
    cout << "\tEncryption:  " << (int) header.encryption << endl; 
    cout << "\tCompression: " << (int) header.compression << endl; 
    cout << "\tPayload size:" << header.payloadSize << endl; 
}





/*****************************************************************************
 *                              Message Encoding                             *
 *****************************************************************************/

int Message::describeData(int senderID,
                          int receiverID,
                          MSGType type,
                          MSGEncryption enc,
                          MSGCompression cmp) {
    header.senderID = senderID;
    header.receiverID = receiverID;
    header.type = type;
    header.encryption = enc;
    header.compression = cmp;
    header.payloadSize = sizeA;
    return 0;
}

int Message::encodeMessage() {
    printHeader();
    if (!isEncode) {
        cerr << "ERROR! Trying to encode a message that is already encoded!" << endl;
        return -1;
    }

    if (header.encryption != MSGEncryption::NONE && encryptionKey == 0) {
        cerr << "WARNING! encrypting a message using the default key: 0" << endl;
    }

    if (encryptMessage()) { cerr << "ERROR! encrypt message failed!" << endl; }
    if (compressMessage()) { cerr << "ERROR! compress message failed!" << endl; }
    if (calculateChecksum()) { cerr << "ERROR! checksum calculation failed!" << endl; }
    if (prepareOutput()) { cerr << "ERROR! unable to prepare output!" << endl; }

    // cout << sizeO << endl;
    // printHexDump(bufO, sizeO);

    return 0;
}




/*****************************************************************************
 *                              Message Decoding                             *
 *****************************************************************************/

int Message::decodeMessage() {
    if (isEncode) {
        cerr << "ERROR! Trying to decode a message that is already decoded!" << endl;
        return -1;
    }

    if (getHeader()) { cerr << "ERROR! Unable to read header of the message" << endl; }
    if (decompressMessage()) { cerr << "ERROR! decompress message failed" << endl; }
    if (decryptMessage()) { cerr << "ERROR! decrypt message failed" << endl; }
    if (preparePayload()) { cerr << "ERROR! payload prep failed" << endl; }
        


    return 0;
    
}

int Message::getSenderID() {
    return 0;
}

int Message::getReceiverID() {
    return 0;
}

MSGType Message::getType() {
    return MSGType();
}

MSGCompression Message::getCompression() {
    return MSGCompression();
}

MSGEncryption Message::getEncryption() {
    return MSGEncryption();
}


void printHexDump(void* buf, size_t size) {
    unsigned char* data = static_cast<unsigned char*>(buf);

    for (size_t i = 0; i < size; i += 16) {
        // Print the offset in hex
        std::cout << std::setw(8) << std::setfill('0') << std::hex << i << ": ";

        // Print hex bytes
        for (size_t j = 0; j < 16; ++j) {
            if (i + j < size) {
                std::cout << std::setw(2) << static_cast<unsigned>(data[i + j]) << " ";
            } else {
                std::cout << "   "; // Fill space for alignment
            }
        }

        // Print ASCII characters
        std::cout << " |";
        for (size_t j = 0; j < 16; ++j) {
            if (i + j < size) {
                char c = data[i + j];
                if (std::isprint(static_cast<unsigned char>(c))) {
                    std::cout << c;
                } else {
                    std::cout << '.';
                }
            }
        }
        std::cout << "|" << std::endl;
    }

    // Reset the formatting
    std::cout << std::dec;
}