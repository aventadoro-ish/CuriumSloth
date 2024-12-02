#include "MessageManager.h"
#include <stdlib.h>     /* srand, rand */
#include <iostream>
#include "utils.h"


#include "CmS_Sound.h"

// #define MM_DEBUG_ENABLE 

#define MAX_TRANSMIT_ATTEMPTS   10

using namespace std;


MessageManger::MessageManger(int senderID, size_t max_msg_size) {
    this->senderID = senderID;
    max_message_size = max_msg_size;

    transmit_attempts = 0;
    txState = MMTransmitState::WAIT_INPUT;
    active_message = nullptr;

    send_queue = QueueProper<Message>();
    receive_queue = QueueProper<Message>();
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
    cout << "Total payload size is 0x" << hex << bufLen << dec << endl; 
    size_t sendingSize = 0;

    // round up int division
    int num_msg = (bufLen + max_message_size - 1) / max_message_size;

    // last iteration should eat the rest of the message
    // may be less than max_message_size bytes long
    for (int i = 0; i < num_msg - 1; i++) {
        Message* msg = new Message();
        sendingSize += max_message_size;
        void* start_idx = (void*)((unsigned long int)buf + (i * max_message_size));
        
#ifdef MM_DEBUG_ENABLE
        cout << "Adding message from @" << hex << start_idx << endl;
#endif // MM_DEBUG_ENABLE
        
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

    }

    Message* msg = new Message();
    void* start_idx =
        (void*)((unsigned long int)buf + ((num_msg - 1) * max_message_size));
    size_t len = bufLen - ((num_msg - 1) *
                         max_message_size);  // TODO: check for off-by-1 error
    
    sendingSize += len;

   
    msg->addData(start_idx, len);
    msg->describeData(senderID, receiverID,
                     generateMsgID(), 
                     type,
                     MSGEncryption::NONE,  // TODO: add a way to change that
                     MSGCompression::NONE  // ... and this too ...
    );
    msg->encodeMessage();

    send_queue.push(msg);

    cout << "Encoded size = " << sendingSize << endl;

    return 0;
}


int MessageManger::tick() {
    switch (txState) {
    case MMTransmitState::WAIT_INPUT:
        if (!send_queue.isEmpty()) txState = MMTransmitState::WAIT_TO_SEND;
        break;
    
    case MMTransmitState::WAIT_TO_SEND:
        if (active_message == nullptr && port->canWrite()) {
            txState = MMTransmitState::READY_TO_SEND;
        }

        if (active_message == nullptr && send_queue.isEmpty()) {
            txState = MMTransmitState::WAIT_INPUT;
        }
        break;
    
    case MMTransmitState::READY_TO_SEND:
        transmitMessage();
        txState = MMTransmitState::AWAITING_ACK;
        break;

    case MMTransmitState::AWAITING_ACK: {
        auto now = std::chrono::steady_clock::now();
        unsigned int timeToTransmit = 
            port->getTimeToTransmitNBytesMs(active_message->getMessageSize()) * 5;

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_transmission).count();
        if (elapsed >= timeToTransmit) {

#ifdef MM_DEBUG_ENABLE
            cout << "\t AWAIT ACK timed out.." << endl;
#endif // MM_DEBUG_ENABLE
            txState = MMTransmitState::WAIT_TO_RESEND;
        }
        
        // receiveing ACK or NACK is handled in MessageManger::tickReceive()
        break;
    }
    case MMTransmitState::WAIT_TO_RESEND:
        if (port->canWrite()) {
            retransmitMessage();
            txState = MMTransmitState::AWAITING_ACK;
        }
        break;
    default:
        break;
    }


    // This method of receiving messages only works for blocking reads
    if (port->numInputBytes() > 0) {
        tickReceive();
    }

  
    return !(send_queue.isEmpty() && active_message == nullptr);
}

void MessageManger::tickReceive() {
    // assumes there are bytes to read

    void* buf = (void*)malloc(getBufferSize());
    size_t rxBytes = 0;

    port->receiveMessage(buf, getBufferSize(), &rxBytes);

    Message* incoming = new Message();
    incoming->addData(buf, rxBytes, false);
    int decodeRes = incoming->decodeMessage();
    
    if (decodeRes != 0) {
        // if decode failed, transmit NACK
#ifdef MM_DEBUG_ENABLE
        cout << "\t\t\tERROR RECEIVE ERROR" << endl;
#endif // MM_DEBUG_ENABLE

        if (active_message == nullptr) {
            sleep_ms(port->getTimeoutMs());
            // cout << "wait timeout ms " << dec << port->getTimeoutMs();
            sendAckState(MSGSystemMessages::NACK);
        }

        return;
    }

    
    // assume a valid message from here
    switch (incoming->getType()) {
    case MSGType::SYSTEM: {
        MSGSystemMessages sys_mes = *((MSGSystemMessages*) incoming->getMessage());
#ifdef MM_DEBUG_ENABLE
        cout << "\tSYSTEM MESSAGE";
#endif // MM_DEBUG_ENABLE

        if (txState == MMTransmitState::AWAITING_ACK) {
#ifdef MM_DEBUG_ENABLE
            cout << "\tRecevieed ack state:" << (int)sys_mes << endl;
#endif // MM_DEBUG_ENABLE
            if (sys_mes == MSGSystemMessages::ACK) {
                txState = MMTransmitState::WAIT_TO_SEND;
                free(active_message);
                active_message = nullptr;

            } else if (sys_mes == MSGSystemMessages::NACK) {
                txState == MMTransmitState::WAIT_TO_RESEND;
            }

        }
    
        break;
    }
    case MSGType::AUDIO:
        cout << "RECEIVING AUDIO PACKET" << endl;
        // printHexDump(incoming->getMessage(), incoming->getMessageSize());
        sendAckState(MSGSystemMessages::ACK); 

        break;
    case MSGType::TEXT: {
        char* text = (char*)incoming->getMessage();
        text[incoming->getOriginalSize()] = 0;
        cout << "RECEIVING TEXT MESSAGE [" << incoming->getOriginalSize() << " ]:" << text << endl;

        break;
       sendAckState(MSGSystemMessages::ACK); 
    }
    default:
        break;
    }


    if (received_ids.find(incoming->getID()) != received_ids.end()) {
        // message has been received already
        return;
    }

    received_ids.insert(incoming->getID());
    receive_queue.push(incoming);

}

void MessageManger::sendAckState(MSGSystemMessages state) {
    Message* msg = new Message();
    MSGSystemMessages* buf = (MSGSystemMessages*)malloc(sizeof(MSGSystemMessages));
    *buf = state;
    msg->addData(buf, sizeof(MSGSystemMessages));
    msg->describeData(
        senderID, 
        0,                      // TODO: change to proper receiver ID
        generateMsgID(), 
        MSGType::SYSTEM, 
        MSGEncryption::NONE, 
        MSGCompression::NONE
    );
    msg->encodeMessage();

    port->sendMessage(msg->getMessage(), msg->getMessageSize());
#ifdef MM_DEBUG_ENABLE
    cout << "\tSent ack state:" << (int)state << endl;
#endif // MM_DEBUG_ENABLE
}

void MessageManger::transmitMessage() {
    Message* msg = send_queue.pop();
    active_message = msg;
    transmit_attempts = 0;

    retransmitMessage();

}

void MessageManger::retransmitMessage() {
    if (transmit_attempts >= MAX_TRANSMIT_ATTEMPTS) {
        txState = MMTransmitState::ERROR_STATE;
        cerr << "ERROR! MessageManger::retransmitMessage()" \
        "MAX_TRANSMIT_ATTEMPTS reached ( " << MAX_TRANSMIT_ATTEMPTS <<  ") for" \
        "message @0x" << hex << active_message << dec << endl;
        active_message->printHeader();
        system("pause");   
    }


    cout << "SENDING MESSAGE (attempt: " << transmit_attempts << ")";
    cout << " size is " << hex << active_message->getMessageSize() << dec << endl;
    port->sendMessage(active_message->getMessage(), active_message->getMessageSize());
    
    last_transmission = chrono::steady_clock::now();

    transmit_attempts++;
}

bool MessageManger::hasMessageBeenReceived(unsigned long int newMsgID) {
    
    
    return false;
}

void MessageManger::replayAudio() {
    QueueProper<Message> tempQueue = QueueProper<Message>();


    size_t total_payload_size = 0;
    while(!receive_queue.isEmpty()) {
        Message* msg = receive_queue.pop();
        total_payload_size += msg->getMessageSize();
        tempQueue.push(msg);
    }
    cout << "Total payload size is 0x" << hex << total_payload_size << dec << endl; 
    void* buf = malloc(total_payload_size);
    

    size_t idx = 0;
    while (!tempQueue.isEmpty()) {
        Message* msg = tempQueue.pop();

        memcpy((void*)((size_t)buf + idx), msg->getMessage(), msg->getMessageSize());
        receive_queue.push(msg);
        idx += msg->getMessageSize();
    }
    
    AudioRecorder arp = AudioRecorder();
    arp.setBuffer(buf, total_payload_size);
    arp.replayAudio();


}



