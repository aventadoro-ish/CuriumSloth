#pragma once
#include <unordered_set>
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


    std::unordered_set<unsigned long int> received_ids;
    QueueProper<Message> receive_queue;
    QueueProper<Message> to_process_queue;

    // awaiting confirmation (retransmit based on timeout)
    // simpler scheme with 1 active message at a time was used instead
    // QueueProper<Message> pending_confirmation_queue;

    int senderID;

    unsigned long generateMsgID();

    COMPort* port;

    WAVEHeader* waveHdr; // used for ADPCM compression

    void tickReceive();
    void sendAckState(MSGSystemMessages state);
    

    void transmitMessage();
    void retransmitMessage();

    void processRollingAudioRx();

    bool hasMessageBeenReceived(unsigned long int newMsgID);
    

public:
    bool isRollingReceive = false;

    MessageManger(int senderID, size_t max_msg_size = 0x3fff);
    ~MessageManger();


    /// @brief Transmit data to a certain receiver. Copies data, splits data
    /// into chunks (if needed), queues up the transmission of data
    /// @param receiverID who should receive the message
    /// @param type data type
    /// @param buf pointer to data
    /// @param bufLen data length in bytes
    /// @param compression compression to be used
    /// @param encryption encryption to be used
    /// @return 0 if successfully added to the transmission queue
    int transmitData(int receiverID, 
                     MSGType type, 
                     void* buf, 
                     size_t bufLen, 
                     MSGCompression compression = MSGCompression::NONE, 
                     MSGEncryption encryption = MSGEncryption::NONE);


    /// @brief Transmits audio data callback
    /// assumes ADPCM compression, no encryption
    /// @param buf 
    /// @param bufLen if 0, only tick() is called
    /// @return 
    int transmitDataRollingCallback(void* buf, size_t bufLen);


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

    /// @brief Copies a waveHeader into a member variable
    /// @param waveHeader 
    void setWaveHeader(WAVEHeader* waveHeader);

    size_t getMaxMessageSize() {
        return max_message_size;
    }


};
