#include <iostream>
#include <string>
#include "Compression.h"
#include "Encryption.h"
#include "COMPort.h"

void testTextMessageWithOptions(const std::string& txPortName, const std::string& rxPortName) {
    std::string message;
    char enableCompression, enableEncryption;

    // Prompt user for the message
    std::cout << "Write your message: ";
    std::cin.ignore(); // Clear any previous input
    std::getline(std::cin, message);

    // Ask if RLE compression is needed
    std::cout << "Enable RLE compression? (y/n): ";
    std::cin >> enableCompression;

    // Ask if XOR encryption is needed
    std::cout << "Enable XOR encryption? (y/n): ";
    std::cin >> enableEncryption;

    // Clear input buffer
    std::cin.ignore();

    // Process the message
    std::string processedMessage = message;
    unsigned char compressedMessage[1024];
    size_t compressedLength = 0;

    if (enableCompression == 'y' || enableCompression == 'Y') {
        compressedLength = RLEcompress((unsigned char*)processedMessage.c_str(),
                                       processedMessage.size(),
                                       compressedMessage,
                                       sizeof(compressedMessage));
        processedMessage = std::string((char*)compressedMessage, compressedLength);
        std::cout << "Compressed Message: " << processedMessage << "\n";
    }

    if (enableEncryption == 'y' || enableEncryption == 'Y') {
        void* encrypted = XORencrypt((void*)processedMessage.c_str(),
                                     processedMessage.size(),
                                     'K'); // Use 'K' as the encryption key
        processedMessage = std::string((char*)encrypted, processedMessage.size());
        free(encrypted); // Free allocated memory for encryption
        std::cout << "Encrypted Message: " << processedMessage << "\n";
    }

    // Open COM ports
    COMPort senderPort(COMPortBaud::COM_BAUD_9600, CPParity::NONE, 1);
    COMPort receiverPort(COMPortBaud::COM_BAUD_9600, CPParity::NONE, 1);

    if (senderPort.openPort(const_cast<char*>(txPortName.c_str())) != CPErrorCode::SUCCESS) {
        std::cerr << "Error: Failed to open transmitter port: " << txPortName << "\n";
        return;
    }

    if (receiverPort.openPort(const_cast<char*>(rxPortName.c_str())) != CPErrorCode::SUCCESS) {
        std::cerr << "Error: Failed to open receiver port: " << rxPortName << "\n";
        senderPort.closePort();
        return;
    }

    // Send the processed message
    if (senderPort.sendMessage(const_cast<char*>(processedMessage.c_str()), processedMessage.size()) == CPErrorCode::SUCCESS) {
        std::cout << "Message sent successfully.\n";
    } else {
        std::cerr << "Error: Failed to send message.\n";
    }

    // Receive the message
    char buffer[1024] = {0};
    size_t bytesRead = 0;

    if (receiverPort.receiveMessage(buffer, sizeof(buffer), &bytesRead, 5000) == CPErrorCode::SUCCESS) {
        std::cout << "Message received successfully.\n";
    } else {
        std::cerr << "Error: Failed to receive message.\n";
    }

    std::string receivedMessage(buffer, bytesRead);

    // Reverse the processing
    if (enableEncryption == 'y' || enableEncryption == 'Y') {
        receivedMessage = XORdecrypt(receivedMessage, 'K'); // Decrypt message
        std::cout << "Decrypted Message: " << receivedMessage << "\n";
    }

    if (enableCompression == 'y' || enableCompression == 'Y') {
        unsigned char decompressedMessage[1024];
        size_t decompressedLength = RLEdecompress((unsigned char*)receivedMessage.c_str(),
                                                  receivedMessage.size(),
                                                  decompressedMessage,
                                                  sizeof(decompressedMessage),
                                                  0x1B); // Escape character
        receivedMessage = std::string((char*)decompressedMessage, decompressedLength);
        std::cout << "Decompressed Message: " << receivedMessage << "\n";
    }

    // Show the final received message
    std::cout << "Original Message: " << receivedMessage << "\n";

    // Close COM ports
    senderPort.closePort();
    receiverPort.closePort();

    system("pause");
}
