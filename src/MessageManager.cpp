#include "MessageManager.h"
#include <stdlib.h>     /* srand, rand */
#include <iostream>


using namespace std;


MessageManger::MessageManger(int senderID, size_t max_msg_size) {
    this->senderID = senderID;
    max_message_size = max_msg_size;
}

MessageManger::~MessageManger() {
    // TODO: add memory clean up
}


unsigned long MessageManger::generateMsgID() {
    // rand is int, so we need to call it twice to get long int
    unsigned long int res = (rand() << (sizeof(int) / 8)) | rand();
    return res;
}

int MessageManger::transmitData(int receiverID,
                                MSGType type,
                                void* buf,
                                size_t bufLen) {
    int num_encode_faults = 0;
    int num_add_faults = 0;

    // round up int division
    int num_msg = (bufLen + max_message_size - 1) / max_message_size;

    // last iteration should eat the rest of the message
    // may be less than max_message_size bytes long
    for (int i = 0; i < num_msg - 1; i++) {
        Message* msg = new Message();

        void* start_idx = (void*)((unsigned long int)buf + (i * max_message_size));
        cout << "Adding message from @" << hex << start_idx << endl;
        // for (int ii = 0; ii < max_message_size; ii++) {
        //     char* temp = (char*)start_idx;
        //     cout << temp[ii];
        // }
        // cout << endl;

        if (msg->addData(start_idx, max_message_size)) {
            num_add_faults++;
        }
        


        msg->describeData(senderID, 
                        receiverID,
                        generateMsgID(),
                        type,
                        MSGEncryption::NONE,  // TODO: add a way to change that
                        MSGCompression::NONE  // ... and this too ...
        );

        if (msg->encodeMessage()) {
            num_encode_faults++;
        }

        // TODO: change queue to menage memory
        send_queue.push(msg);
        
        // delete msg;

        cout << "encoded message chuck " << i << endl;
    }

    Message* msg = new Message();
    void* start_idx =
        (void*)((unsigned long int)buf + ((num_msg - 1) * max_message_size));
    size_t len = bufLen - ((num_msg - 1) *
                         max_message_size);  // TODO: check for off-by-1 error
    msg->addData(start_idx, len);
    msg->describeData(senderID, receiverID,
                     generateMsgID(), 
                     type,
                     MSGEncryption::NONE,  // TODO: add a way to change that
                     MSGCompression::NONE  // ... and this too ...
    );
    msg->encodeMessage();

    send_queue.push(msg);

    // delete msg;
    return 0;
}


int MessageManger::tick() {
    // if there is stuff to send and ouput buffer is empty
    // if (!send_queue.isEmpty() && port->numOutputButes() == 0) {
    if (!send_queue.isEmpty() && port->canWrite()) {
        Message* msg = send_queue.pop();

        cout << "SENDING MESSAGE:";
        // msg->printHeader();

        char* msg_payload = (char*)msg->getMessage() + sizeof(MSGHeader);

        for (int i = 0; i < msg->getPayloadSize(); i++) {
            cout << msg_payload[i];
        }
        cout << endl;

        port->sendMessage(msg->getMessage(), msg->getMessageSize());
        
        free(msg);
    }

    // This method of receiving messages only works for blocking reads
    if (port->numInputBytes() > 0) {
        // >= getBufferSize()
        void* buf = (void*)malloc(getBufferSize());

        port->receiveMessage(buf, getBufferSize());

        Message incoming = Message();
        incoming.addData(buf, getBufferSize(), false);
        incoming.decodeMessage();

        char* text = (char*)incoming.getMessage();
        text[incoming.getOriginalSize()] = 0;
        cout << "RECEIVING MESSAGE [" << incoming.getOriginalSize() << " ]:" << text << endl;
    }

  
    return !send_queue.isEmpty();
}




