#include "MessageManager.h"
#include <stdlib.h>     /* srand, rand */
#include <iostream>

#include "CmS_Sound.h"

using namespace std;


MessageManger::MessageManger(int senderID, size_t max_msg_size) {
    this->senderID = senderID;
    max_message_size = max_msg_size;

    send_queue = QueueProper<Message>();
    in_queue = QueueProper<Message>();
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

    return 0;
}


int MessageManger::tick() {
    // if there is stuff to send and ouput buffer is empty
    // if (!send_queue.isEmpty() && port->numOutputBytes() == 0) {
    if (!send_queue.isEmpty() && port->canWrite()) {
        Message* msg = send_queue.pop();

        cout << "SENDING MESSAGE:";
        // msg->printHeader();

        // char* msg_payload = (char*)msg->getMessage() + sizeof(MSGHeader);
        // for (int i = 0; i < msg->getPayloadSize(); i++) {
        //     cout << msg_payload[i];
        // }
        // cout << endl;
        cout << " size is " << hex << msg->getMessageSize();
        port->sendMessage(msg->getMessage(), msg->getMessageSize());
        
        free(msg); // TODO: bring back
    }

    // This method of receiving messages only works for blocking reads
    if (port->numInputBytes() > 0) {
        // >= getBufferSize()
        void* buf = (void*)malloc(getBufferSize());

        port->receiveMessage(buf, getBufferSize());

        Message* incoming = new Message();
        incoming->addData(buf, getBufferSize(), false);
        incoming->decodeMessage();

        in_queue.push(incoming);

        if (incoming->getType() == MSGType::AUDIO) {
            if (incoming->getSenderID() != 0 ||
                incoming->getReceiverID() != 1) {
                    cout << "fault************************" << endl;
                }


        } else if (incoming->getType() == MSGType::TEXT) {
            char* text = (char*)incoming->getMessage();
            text[incoming->getOriginalSize()] = 0;
            cout << "RECEIVING TEXT MESSAGE [" << incoming->getOriginalSize() << " ]:" << text << endl;

        }

    }

  
    return !send_queue.isEmpty();
}


void MessageManger::replayAudio() {
    QueueProper<Message> tempQueue = QueueProper<Message>();


    size_t total_payload_size = 0;
    while(!in_queue.isEmpty()) {
        Message* msg = in_queue.pop();
        total_payload_size += msg->getMessageSize();
        tempQueue.push(msg);
    }

    void* buf = malloc(total_payload_size);
    

    size_t idx = 0;
    while (!tempQueue.isEmpty()) {
        Message* msg = tempQueue.pop();

        memcpy((void*)((size_t)buf + idx), msg->getMessage(), msg->getMessageSize());
        in_queue.push(msg);
    }
    
    AudioRecorder arp = AudioRecorder();
    arp.setBuffer(buf, total_payload_size);
    arp.replayAudio();


}



