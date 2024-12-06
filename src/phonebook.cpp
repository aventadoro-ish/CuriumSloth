#include "phonebook.h"
#include "COMPort.h"
#include "MessageManager.h"
#include "terminal.h"
#include <iostream>

using namespace std;

extern std::string txPortName; // Transmitter port set at startup
extern std::string rxPortName; // Receiver port set at startup

Phonebook::Phonebook() {
    for (int i = 0; i < MAX_IDS; i++) {
        phonebook[i] = nullptr;
    }
}

Phonebook::~Phonebook() {
    for (int i = 0; i < MAX_IDS; i++) {
        PhonebookEntry* current = phonebook[i];
        while (current) {
            PhonebookEntry* temp = current;
            current = current->next;
            delete temp;
        }
    }
}

void Phonebook::addMessage(int id, const std::string& message, bool isReceiver) {
    if (id < 0 || id >= MAX_IDS) {
        cerr << "Invalid ID range!" << endl;
        return;
    }

    PhonebookEntry* newEntry = new PhonebookEntry(id, message);
    if (!phonebook[id]) {
        phonebook[id] = newEntry;
    }
    else {
        PhonebookEntry* current = phonebook[id];
        while (current->next) {
            current = current->next;
        }
        current->next = newEntry;
    }
}

void Phonebook::displayMessages(int id, bool byReceiver) {
    if (id < 0 || id >= MAX_IDS || !phonebook[id]) {
        cout << "No messages found!" << endl;
        return;
    }

    PhonebookEntry* current = phonebook[id];
    cout << (byReceiver ? "Messages Received by " : "Messages Sent by ")
        << id << ":\n";

    while (current) {
        cout << "- " << current->message << endl;
        current = current->next;
    }

    system("pause");
}

void Phonebook::viewMessagesByReceiver(int receiverID) {
    displayMessages(receiverID, true);
}

void Phonebook::viewMessagesBySender(int senderID) {
    displayMessages(senderID, false);
}

void Phonebook::testSendReceive() {
    if (txPortName.empty() || rxPortName.empty()) {
        std::cerr << "Error: COM ports are not configured. Please set them first.\n";
        return;
    }

    string message;
    int senderID, receiverID;

    cout << "Enter Sender ID: ";
    cin >> senderID;
    cout << "Enter Receiver ID: ";
    cin >> receiverID;

    cout << "Enter Message: ";
    cin.ignore(); // Clear newline
    getline(cin, message);

    // Open COM ports
    COMPort senderPort(COMPortBaud::COM_BAUD_9600, CPParity::NONE, 1);
    COMPort receiverPort(COMPortBaud::COM_BAUD_9600, CPParity::NONE, 1);

    if (senderPort.openPort(const_cast<char*>(txPortName.c_str())) != CPErrorCode::SUCCESS) {
        std::cerr << "Error: Failed to open transmitter port: " << txPortName << "\n";
        return;
    }

    if (receiverPort.openPort(const_cast<char*>(rxPortName.c_str())) != CPErrorCode::SUCCESS) {
        std::cerr << "Error: Failed to open receiver port: " << rxPortName << "\n";
        senderPort.closePort(); // Ensure cleanup
        return;
    }

    // Send the message
    if (senderPort.sendMessage(const_cast<char*>(message.c_str()), message.size()) == CPErrorCode::SUCCESS) {
        std::cout << "Message sent successfully.\n";
    } else {
        std::cerr << "Error sending message.\n";
    }

    // Receive the message
    char buffer[1024] = {0};
    size_t bytesRead = 0;
    if (receiverPort.receiveMessage(buffer, sizeof(buffer), &bytesRead, 5000) == CPErrorCode::SUCCESS) {
        std::cout << "Received Message: " << string(buffer, bytesRead) << "\n";
    } else {
        std::cerr << "Error receiving message.\n";
    }

    // Store messages in the phonebook
    addMessage(senderID, message, false);
    addMessage(receiverID, string(buffer, bytesRead), true);

    // Close COM ports
    senderPort.closePort();
    receiverPort.closePort();
}



void Phonebook::menu() {
    int choice;
    do {
        clearScreen();
        cout << "\n===== Phonebook Menu =====\n";
        cout << "1. View Messages by Receiver ID\n";
        cout << "2. View Messages by Sender ID\n";
        cout << "3. Test Send/Receive Message\n";
        cout << "0. Back to Main Menu\n";
        cout << "==========================\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
        case 1: {
            int receiverID;
            cout << "Enter Receiver ID: ";
            cin >> receiverID;
            viewMessagesByReceiver(receiverID);
            break;
        }
        case 2: {
            int senderID;
            cout << "Enter Sender ID: ";
            cin >> senderID;
            viewMessagesBySender(senderID);
            break;
        }
        case 3:
            testSendReceive();
            system("pause");
            break;
        case 0:
            break;
        default:
            cout << "Invalid choice! Try again.\n";
        }
    } while (choice != 0);
}