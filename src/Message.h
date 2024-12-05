/*****************************************************************************
 * This class manages creating and modifying a message. It handles:
 *  - message data allocation
 *  - message encryption
 *  - message compression
 *  - header forming
 *****************************************************************************/

#pragma once
#include <stdio.h>	
#include "adpcm.h"


enum class MSGType : unsigned short {
    TEXT,       // for plain text ascii
    AUDIO,      // for audio
    SYSTEM      // for system events, e.g. receive acknowledge, audio settings sync
};

enum class MSGEncryption : unsigned short {
    NONE,
    AES,
    XOR
};

enum class MSGCompression : unsigned short {
    NONE,
    HUFFMAN,
    RLE,
    ADPCM
};

enum class MSGSystemMessages {
    ACK,
    NACK,
    WAVEHEADER
};


struct MSGHeader {
    short senderID;
    short receiverID;
    unsigned long int messageID;
    MSGType type;
    MSGEncryption encryption;
    MSGCompression compression;
    unsigned int decompressedSize;
    unsigned int payloadSize;   // in bytes
    short checksum;

    MSGHeader() {
        senderID = 0;
        receiverID = 0;
        type = MSGType::TEXT;
        encryption = MSGEncryption::NONE;
        compression = MSGCompression::NONE;
        decompressedSize = 0;
        payloadSize = 0;
        checksum = 0;
    };
};


class Message {
    /* Class operation overview (encoding)
     * 1. Pass a message buffer addData(..., ..., true)
     * 2. A copy of the buffer is allocated (bufA)
     * 3. Message is encrypted (bufB)
     * 4. Message is compressed (bufC)
     * 5. Header is generated
     * 6. Raw bytes can be transmitted
     * 
     * Class operation overview (decoding)
     * 1. Pass a message buffer addData(..., ..., false)
     * 2. A copy of the buffer is allocated (bufA)
     * 3. Header is read from the message to establish message parameters
     * 4. Message is decompressed into bufB
     * 5. Message is decrypted into bufC
     * 6. Payload and message info can be read
     */

    void* bufA;         // Original message buffer (copy of the passed buffer)
    size_t sizeA;       // Original message buffer size

    void* bufB;         // Encrypted message buffer (copy of the passed buffer)
    size_t sizeB;       // Encrypted message buffer size
    
    void* bufC;         // Encrypted and compressed message buffer (copy of the passed buffer)
    size_t sizeC;       // Encrypted and compressed message buffer size

    void* bufO;         // Output message buffer (encoded with header and footer)
    size_t sizeO;       // Output message buffer size

    WAVEHeader* waveHeader;

    MSGHeader header;

    char* encryptionKey;
    size_t eKeyLen;
    bool isEncode;

    /// @brief Computes/validates header member. In encode mode, checksum is
    /// calculated and added to the header. In decode mode, checksum is calculated
    /// and compared to the checksum of the received header.
    /// @return false in header is corrupted. true otherwise
    bool validateHeader();

    int encryptMessage();
    int compressMessage();

    // performs compression with type NONE
    void compressNone();
    void compressADPCM();

    int prepareOutput();

    int getHeader();
    int decompressMessage();
    int decryptMessage();
    int preparePayload();

    int handleSystemMessage();



public:
    Message();
    ~Message();

    /// @brief Pass message data to the class. A copy of the message is 
    /// allocated by the class
    /// @param buf message to copy
    /// @param size size of the message in bytes
    /// @param encode operation on data. true -> encode, false -> decode
    /// @return 0 if success
    int addData(void* buf, size_t size, bool encode = true);


    /// @brief Copies the provided key into encryptionKey member that is used 
    /// to encrypt a message. If encryption is NONE, this is not needed. \r
    /// @param key key to be copied into memory
    /// @param keyLen length of the key array
    /// @return 0 if success
    int setEncryptionKey(char* key, size_t keyLen);


    /// @brief For ENCODING: Returns a pointer to the buffer with encoded raw 
    /// message data (header + payload + footer) \n
    /// For DECODING: Returns a pointer to the buffer with payload 
    /// (decompressed, decrypted, and stripped of header and footer)
    /// @return buffer pointer or nullptr if failed
    void* getMessage();

    /// @brief For ENCODING: Returns the number of bytes taken up by the 
    /// encoded message (header + payload + footer) \n
    /// For DECODING: Returns the number of bytes taken up by the payload 
    /// (decompressed, decrypted, and stripped of header and footer)
    /// @return number of bytes or 0 if failed
    size_t getMessageSize();

    /// @brief Provide WaveHeader data for audio compression and decompression
    /// @param hdr 
    void setWaveHeader(WAVEHeader hdr);


    void printHeader();


    /*************************************************************************
     *                          Message Encoding                             *
     *************************************************************************/

    /// @brief Allows to configure the handling of the message for encoding
    /// @param senderID who is sending the message
    /// @param receiverID who is receiving the message
    /// @param messageID allows to distinguish messages
    /// @param type type of data
    /// @param enc encryption standard to use
    /// @param cmp compression standard to use
    /// @return 0 if success
    int describeData(
        int senderID, 
        int receiverID, 
        unsigned long int messageID,
        MSGType type, 
        MSGEncryption enc, 
        MSGCompression cmp
    );

    /// @brief Attempts to encode the message: encrypt, compress, add header, 
    /// add checksum
    /// @return 0 if success
    int encodeMessage();


    /*************************************************************************
     *                          Message Decoding                             *
     *************************************************************************/

    /// @brief Attempts to decode a raw message. Extracts data from header, 
    /// decompresses, decrypts the message, verifies the checksum
    /// @return 0 if success
    int decodeMessage();



    int getSenderID();

    int getReceiverID();

    MSGType getType();

    MSGCompression getCompression();

    MSGEncryption getEncryption();

    unsigned int getOriginalSize();

    unsigned int getPayloadSize();

    unsigned long int getID();

};