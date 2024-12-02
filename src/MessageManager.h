#pragma once

#include "Message.h"
#include "Queue2.h"
#include "COMPort.h"

enum class MMTransmitState {
    WAIT_INPUT,     // wait for new data to transmit 
                    // send_queue is empty and active_file is nullptr
    WAIT_TO_SEND,   // wait for output buffer to be clear and write timeout to occur
    READY_TO_SEND,  // ready to transmit a message
    AWAITING_ACK,   // waiting for ACK, NACK or timeout
    WAIT_TO_RESEND, // waiting for retransmit a corrupted message
    SEND_ACK,
    SEND_NACK,

    ERROR_STATE,
    num_states
};


/// @brief MessageManger handles sending data to a receiver. It splits
/// long data into chunks/multiple messages, queues up messages to be sent,
/// awaits confirmation. \n
/// 
/// TODO: add the ability to hook up a stream of data to be sent (real time transmission)
class MessageManger {
    // determines how to split messages
    size_t max_message_size;

    int transmit_attempts;
    MMTransmitState txState;
    Message* active_message;
    std::chrono::steady_clock::time_point last_transmission;

    // awaiting sending (first in, first out)
    QueueProper<Message> send_queue;

    QueueProper<Message> in_queue;

    // awaiting confirmation (retransmit based on timeout)
    QueueProper<Message> pending_confirmation_queue;

    int senderID;

    unsigned long generateMsgID();

    COMPort* port;

    void tickReceive();
    void sendAckState(MSGSystemMessages state);
    

    void transmitMessage();
    void retransmitMessage();
    

public:
    MessageManger(int senderID, size_t max_msg_size = 0x3fff);
    ~MessageManger();


    /// @brief Transmit data to a certain receiver. Copies data, splits data
    /// into chunks (if needed), queues up the transmission of data
    /// @param receiverID who should receive the message
    /// @param type data type
    /// @param buf pointer to data
    /// @param bufLen data length in bytes
    /// @return 0 if successfully added to the transmission queue
    int transmitData(int receiverID, MSGType type, void* buf, size_t bufLen);


    /// @brief Needs to be called in the main loop
    /// @return 0 if all queues are empty, -1 if error, 1 if queues are not empty (no errors)
    int tick();

    /// @brief Temp function that combines incoming queue to play it as a single audio message
    void replayAudio();

    void setCOMPort(COMPort* port) {
        this->port = port;
    }

    size_t getBufferSize() {
        return max_message_size + sizeof(MSGHeader);
    }


};
