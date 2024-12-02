#include "Message.h"
#include <iostream>
#include <stdlib.h>	
#include <cstring>	
#include <iomanip>
#include <cstdint>

#include "Compression.h"
#include "Encryption.h"
#include "utils.h"

using namespace std;

// #define MESSAGE_DEBUG_PRINTS

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

  encryptionKey = nullptr;
  isEncode = true;

  header = MSGHeader();
}

Message::~Message() {
    if (bufA != nullptr) { free(bufA); }
    if (bufB != nullptr) { free(bufB); }
    if (bufC != nullptr) { free(bufC); }
    if (bufO != nullptr) { free(bufO); }
    if (encryptionKey != nullptr) { free(encryptionKey); }

}

bool Message::validateHeader() {
    short* ptr = (short*)&header;
    short sum = 0;
    cout << hex;
    // for all fields except for checksum field
    const int num_fields = sizeof(MSGHeader) / sizeof(short) - sizeof(short);
    for (int i = 0; i < num_fields; i++) {
        sum += *ptr; 
        ptr++;
    }
    
    if (isEncode) {
        header.checksum = sum;
        return true;
    }

    // decoding
    return header.checksum == sum;
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

    case MSGEncryption::XOR: {
        bufB = XORencrypt(bufA, sizeA, *encryptionKey);
        sizeB = sizeA;
        // printHexDump(bufB, sizeB);
        
        break;
    }
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
    case MSGCompression::RLE: {
        bufC = (void*)malloc(sizeB);

        sizeC = RLEcompress((unsigned char*)bufB, sizeB, (unsigned char*)bufC, sizeB);
        // cout << "Compressed message: ";
        // for (int i = 0; i < sizeC; i++) { cout << ((char*)bufC)[i]; } cout << endl;

        // cout << "Compressed size: " << sizeC << endl;
        // int count = 0;
        // for (int i = 0; i < sizeC; i++) { count += ((char*)bufC)[i] == 0x1b; }
        // cout << "Num escape chars: " << count << endl;
        // printHeader();
        break;
    }
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

int Message::prepareOutput() {
    if (bufC == nullptr) {
        cerr << "ERROR! Can't prepare an nullptr buffer" << endl;
        return -1;
    }

    if (bufO != nullptr) {
        cerr << "WARNING! Overriding bufO" << endl;
        return -1;
    }

    header.payloadSize = sizeC;
    validateHeader();

    size_t total_size = sizeof(MSGHeader) + sizeC;
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
  
    void* payloadStartIdx = (void*)((unsigned long int)bufO + sizeof(MSGHeader));
    memcpy(payloadStartIdx, bufC, sizeC);

    memcpy(bufO, &header, sizeof(MSGHeader));
    

    return 0;
}





int Message::getHeader() {
    if (bufA == nullptr) {
        cerr << "ERROR! Trying to read header from nullptr buffer" << endl;
        return -1;
    }

    memcpy(&header, bufA, sizeof(MSGHeader));
    if (!validateHeader()) {
        return -1;  // invalid header
    }

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

    // pointer to the start of the message excluding the header 
    void* messageStartIdx = (void*)((unsigned long int)bufA + sizeof(MSGHeader));

    if (header.payloadSize != sizeA - sizeof(MSGHeader)) {
#ifdef MESSAGE_DEBUG_PRINTS
        cerr << "WARNING! Message::decompressMessage() -> header.payloadSize (" 
             << header.payloadSize << ") != received payload size (" 
             << sizeA - sizeof(MSGHeader) << ")" << endl;
#endif // MESSAGE_DEBUG_PRINTS
        return -1;
    }


    switch (header.compression) {
    case MSGCompression::NONE:
        sizeB = header.payloadSize;
        bufB = (void*)malloc(sizeB);
        memcpy(bufB, messageStartIdx, sizeB);
        break;
    case MSGCompression::RLE:{
        sizeB = header.decompressedSize;
        bufB = (void*)malloc(sizeB);
        

        RLEdecompress((unsigned char*)messageStartIdx, header.payloadSize, (unsigned char*)bufB, sizeB, 0x1b);
        // int count = 0;
        // for (int i = 0; i < sizeB; i++) { count += ((char*)bufB)[i] == 0x1b; }
        // cout << "Num escape chars: " << count << endl;
        // printHexDump(bufB, sizeB);

        // cout << "Compressed message: ";
        // for (int i = 0; i < sizeB; i++) { cout << ((char*)bufB)[i]; } cout << endl;

        break;
    }
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
    case MSGEncryption::XOR: {
        bufC = XORencrypt(bufB, sizeB, *encryptionKey);
        sizeC = sizeB;
        break;
    }
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

    if (sizeO != header.decompressedSize) {
        cerr << "WARNING! Message::preparePayload() -> sizeO != header.decompressedSize" << endl;
    }

    // cout << "Payload message" << endl;
    // printHexDump(bufO, sizeO);

    return 0;
}

int Message::handleSystemMessage() {
    MSGSystemMessages* payloadStart = (MSGSystemMessages*)((size_t)bufA + sizeof(MSGHeader));

    if (*payloadStart == MSGSystemMessages::ACK || 
        *payloadStart == MSGSystemMessages::NACK) {
        sizeO = sizeof(MSGSystemMessages);
        bufO = malloc(sizeO);
        *(MSGSystemMessages*)bufO = *payloadStart;
        return 0;
    }

    // invalid system message
    cerr << "ERROR! Invalid System Message was received: " << (int)*payloadStart << endl;
    return -1;
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

int Message::setEncryptionKey(char* key, size_t keyLen) {
    encryptionKey = (char*)malloc(keyLen * sizeof(char));
    memcpy(encryptionKey, key, keyLen);
    eKeyLen = keyLen;

    // returns 0 for encryptionKey != nullptr => allocated successfully
    return (encryptionKey == nullptr);
}

void* Message::getMessage() {
    return bufO;
}

size_t Message::getMessageSize() {
    return sizeO;
}






void Message::printHeader() {
    cout << "Message header:" << endl;
    cout << "\tSender ID:   " << header.senderID << endl; 
    cout << "\tReceiver ID: " << header.receiverID << endl; 
    cout << "\tMessage ID:  " << header.messageID << endl; 
    cout << "\tEncryption:  "; 
    switch (header.encryption) {
    case MSGEncryption::NONE:       cout << "none";         break;
    case MSGEncryption::AES:        cout << "AES";          break;
    case MSGEncryption::XOR:        cout << "XOR";          break;
    default:                        cout << "unsupported";  break;
    }
    cout << endl;

    cout << "\tCompression: "; 
    switch (header.compression) {
    case MSGCompression::NONE:      cout << "none";         break;
    case MSGCompression::HUFFMAN:   cout << "Huffman";      break;
    case MSGCompression::RLE:       cout << "RLE";          break;
    default:                        cout << "unsupported";  break;
    }
    cout << endl;

    cout << "\tType: "; 
    switch (header.type) {
    case MSGType::AUDIO:            cout << "audio";        break;
    case MSGType::TEXT:             cout << "text";         break;
    case MSGType::SYSTEM:           cout << "system";       break;
    default:                        cout << "unsupported";  break;;
    }
    cout << endl;


    cout << "\tOriginal size:" << header.decompressedSize << endl; 
    cout << "\tPayload size:" << header.payloadSize << endl; 
    cout << "\tChecksum:" << header.checksum << endl; 
}





/*****************************************************************************
 *                              Message Encoding                             *
 *****************************************************************************/

int Message::describeData(int senderID,
                          int receiverID,
                          unsigned long int messageID,
                          MSGType type,
                          MSGEncryption enc,
                          MSGCompression cmp) {
    header.senderID = senderID;
    header.receiverID = receiverID;
    header.messageID = messageID;
    header.type = type;
    header.encryption = enc;
    header.compression = cmp;
    header.decompressedSize = sizeA;
    header.payloadSize = sizeA;
    return 0;
}

int Message::encodeMessage() {
    if (!isEncode) {
        cerr << "ERROR! Trying to encode a message that is already encoded!" << endl;
        return -1;
    }

    if (header.encryption != MSGEncryption::NONE && encryptionKey == 0) {
        cerr << "WARNING! encrypting a message using the default key: 0" << endl;
    }

    if (encryptMessage()) {
        cerr << "ERROR! encrypt message failed!" << endl;
        return -1;
    }
    if (compressMessage()) {
        cerr << "ERROR! compress message failed!" << endl;
        return -1;
    }
    if (prepareOutput()) {
        cerr << "ERROR! unable to prepare output!" << endl;
        return -1;
    }

    // printHeader();

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

    if (getHeader()) { 
        cerr << "ERROR! Unable to read header of the message" << endl;
        return -1;
    }

    if (header.type == MSGType::SYSTEM) {
        return handleSystemMessage();
    }

    if (decompressMessage()) { 
        cerr << "ERROR! decompress message failed" << endl;
        return -1;
    }
    if (decryptMessage()) { 
        cerr << "ERROR! decrypt message failed" << endl;
        return -1;
    }
    if (preparePayload()) { 
        cerr << "ERROR! payload prep failed" << endl;
        return -1;
    }
        

    return 0;
    
}

int Message::getSenderID() {
    return header.senderID;
}

int Message::getReceiverID() {
    return header.receiverID;
}

MSGType Message::getType() {
    return header.type;
}

MSGCompression Message::getCompression() {
    return header.compression;
}

MSGEncryption Message::getEncryption() {
    return header.encryption;
}

unsigned int Message::getOriginalSize() {
    return header.decompressedSize;
}

unsigned int Message::getPayloadSize() {
    return header.payloadSize;
}
