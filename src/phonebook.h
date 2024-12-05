#ifndef PHONEBOOK_H
#define PHONEBOOK_H

#include <string>
#include "Message.h"

struct PhonebookEntry {
    int id;
    std::string message;
    PhonebookEntry* next;

    PhonebookEntry(int id, const std::string& msg)
        : id(id), message(msg), next(nullptr) {}
};

class Phonebook {
private:
    static const int MAX_IDS = 100;
    PhonebookEntry* phonebook[MAX_IDS]; // Array of linked lists

    void displayMessages(int id, bool byReceiver);

public:
    Phonebook();
    ~Phonebook();

    void addMessage(int id, const std::string& message, bool isReceiver);
    void viewMessagesByReceiver(int receiverID);
    void viewMessagesBySender(int senderID);
    void testSendReceive();

    void menu();
};

#endif