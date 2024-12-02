#include "phonebook.h"
#include "COMPort.h"
#include "MessageManager.h"
#include "terminal.h"
#include <iostream>

using namespace std;

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
    int senderPortNum, receiverPortNum;
    string message;
    int senderID, receiverID;

    cout << "Enter Sender Port Number (e.g., 1 for COM1): ";
    cin >> senderPortNum;
    cout << "Enter Receiver Port Number (e.g., 2 for COM2): ";
    cin >> receiverPortNum;

    cout << "Enter Sender ID: ";
    cin >> senderID;
    cout << "Enter Receiver ID: ";
    cin >> receiverID;

    cout << "Enter Message: ";
    cin.ignore(); // Clear newline character from the input buffer
    getline(cin, message);

    // Construct COM port names
    string comPortSender = "COM" + to_string(senderPortNum);
    string comPortReceiver = "COM" + to_string(receiverPortNum);

    // Open COM ports
    COMPort senderPort, receiverPort;
    senderPort.openPort(const_cast<char*>(comPortSender.c_str()));
    receiverPort.openPort(const_cast<char*>(comPortReceiver.c_str()));

    // Send and receive the message
    senderPort.sendMessage(const_cast<char*>(message.c_str()), message.size());
    char buffer[1024] = { 0 };
    receiverPort.receiveMessage(buffer, sizeof(buffer), sizeof(buffer), 5000);

    cout << "Message sent and received successfully.\n";

    // Store messages in the phonebook
    addMessage(senderID, message, false);
    addMessage(receiverID, buffer, true);

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
