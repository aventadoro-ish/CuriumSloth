#include "tests.h"

#include <iostream>
#include <string>
#include <cstring>

#include <adpcm.h>

#include "COMPort.h"
#include "Message.h"
#include "MessageManager.h"
#include "CmS_Sound.h"
#include "utils.h"

#pragma message("Hereeee")


using namespace std;

void testMessageManagerRecSide() {

    // Discards the input buffer
    fflush(stdin);


    // COMPort port = COMPort();
    COMPort port = COMPort(COMPortBaud::COM_BAUD_115200, CPParity::NONE, 1);

    while(port.openPort("/dev/ttyUSB1") != CPErrorCode::SUCCESS) {
        ;
    }
    

    
    MessageManger mngr = MessageManger(1, 5);
    mngr.setCOMPort(&port);

    // while (mngr.tick()) {
    while (true) {
        mngr.tick();

        if (kbhit()) {
            cout << "exiting receive mode" << endl;
            break;
        }
    }


    // size_t buf_size = 500;
    // void* buf = (void*)malloc(buf_size);
    // for (;;) {
    //     if (port.receiveMessage(buf, buf_size) == CPErrorCode::SUCCESS) {
    //         Message msg = Message();
    //         msg.addData(buf, buf_size, false);
    //         msg.decodeMessage();
    //         // cout << (char*)msg.getMessage() << endl;
    //     } else {
    //         cout << "Port read error" << endl;
    //     }
    //     if (kbhit()) {
    //         cout << "Closing program" << endl;
    //         return;
    //     }
    // }

}

void testMessageManagerSendSide() {
    // Discards the input buffer
    fflush(stdin);

    char* test1 = "Heeeeeeeello there my deeeeeeeeeearrrr friend!";

    COMPort port = COMPort(COMPortBaud::COM_BAUD_115200, CPParity::NONE, 1);
    while(port.openPort("/dev/ttyUSB0") != CPErrorCode::SUCCESS) {
        ;
    }
    cout << "Port timeout ms is " << port.getTimeoutMs() << endl;

    MessageManger mngr = MessageManger(0, 5);
    mngr.setCOMPort(&port);


    mngr.transmitData(1, MSGType::TEXT, test1, strlen(test1) + 1);


    cout << endl << endl << endl;


    while (mngr.tick()) {
        // sleep(3);
        // for (unsigned int i = 0; i != 0xfffff; i++) {
        //     ; // wait
        // }
    }

    cout << "Sending ended with no isuuse" << endl;
}


void testMessageManager() {

}


void COMSpeedTest() {
    char ch;
    cout << "Enter t for transmit, r for receive side: ";
    cin >> ch;

    // COMPortBaud baud = COMPortBaud::COM_BAUD_460800; // works
    COMPortBaud baud = COMPortBaud::COM_BAUD_MAX ; //
    size_t bufSize = 0x8fff;
    size_t maxMes = 0x800;

    if (ch == 't') {
        cout << "Transmit side: Openning COM3" << endl;

        // cout << sizeof(MSGHeader) - sizeof(short) << endl;
        COMPort tx = COMPort(baud, CPParity::EVEN, 1);
        tx.openPort("COM3");
        MessageManger txMan = MessageManger(0, maxMes);
        txMan.setCOMPort(&tx);

        cout << "Port is open. Populating buffer" << endl;
    
        auto start = chrono::steady_clock::now();
        cout << "Transmitting " << bufSize / 1024 << " kB in " << maxMes << " chunks" << endl;


        char* buf = (char*) malloc(bufSize);

        for (int i = 0; i < bufSize; i++) {
            buf[i] = (char) i &0xff;
        }

        cout << "Sending data" << endl;

        txMan.transmitData(1, MSGType::AUDIO, buf, bufSize);

        for (;;) {
            if (kbhit()) {
                tx.closePort();
                cout << "exiting" << endl;
                break;
            }

            if (!txMan.tick()) {
                break;
            }
        
        }

        auto end = chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        cout << endl << endl << endl << dec;
        cout << "Transmitted " << bufSize / 1024 << " kB in " << maxMes << " chunks in " << elapsed / 1000.0 << " seconds" << endl;
        cout << "Average transmission rate = " << 1000.0 * (double)bufSize / 1024.0 / elapsed << " kB/s" << endl;

    } else if (ch == 'r') {
        cout << "Receive side: Openning COM8" << endl;

        COMPort rx = COMPort(baud, CPParity::EVEN, 1);
        rx.openPort("COM8");
        MessageManger rxMan = MessageManger(1, maxMes);
        rxMan.setCOMPort(&rx);
        
        cout << "Port is open. Waiting for input" << endl;


        for (;;) {
            if (kbhit()) {
                rx.closePort();
                cout << "exiting" << endl;
                break;
            }

            rxMan.tick();

        }

    } else {

    }

}



void ADPCMCompressionTest() {
    AudioRecorder ar = AudioRecorder();
    
    cout << "Recodring" << endl;
    ar.recordAudio(5);
    cout << "Replay" << endl;
    ar.replayAudio();

    cout << "Compress" << endl;
    WAVEHeader waveHdr = ar.getWaveHeader();

    void* compressedAudio = (void*)malloc(ADPCMDataSize(waveHdr));
    ADPCMHeader adpcmHeader = ADPCMHeader();

    compress((char*)ar.getBuffer(), (char*)compressedAudio, waveHdr, adpcmHeader);

    cout << "Original size: " << ar.getBufferSize() << " Compressed size: " << ADPCMDataSize(waveHdr) << endl;
    
    
    // needed for decompression: waveHdr.subchunk2Size, compressedAudio
    cout << "adpcmHeader.dataSize = " << adpcmHeader.dataSize << endl;
    cout << "adpcmHeader.chunkSize = " << adpcmHeader.chunkSize << endl;
    cout << "adpcmHeader.ch2StepIndex = " << adpcmHeader.ch2StepIndex << endl;

    cout << "Decompressing" << endl;
    void* decompressedAudio = (void*)malloc(waveHdr.subchunk2Size);
    decompress((char*)compressedAudio, (char*)decompressedAudio, adpcmHeader);

    cout << "Replay" << endl;
    AudioRecorder arp = AudioRecorder();
    arp.setBuffer(decompressedAudio, waveHdr.subchunk2Size);
    arp.replayAudio();

    return;
}



void devTesting() {
    char ch;
    cout << "Enter t for transmit, r for receive side: ";
    cin >> ch;

    // COMPortBaud baud = COMPortBaud::COM_BAUD_460800; // works
    COMPortBaud baud = COMPortBaud::COM_BAUD_MAX ; //
    size_t maxMes = 0x800;

    if (ch == 't') {
        cout << "Transmit side: Openning COM3" << endl;

        // cout << sizeof(MSGHeader) - sizeof(short) << endl;
        COMPort tx = COMPort(baud, CPParity::EVEN, 1);
        tx.openPort("COM3");
        MessageManger txMan = MessageManger(0, maxMes);
        txMan.setCOMPort(&tx);

        cout << "Port is open. Populating buffer" << endl;
    

        AudioRecorder ar = AudioRecorder(8000, 16, 1);
        cout << "Record audio" << endl;
        ar.recordAudio(5);
        cout << "Confirm audio" << endl;
        ar.replayAudio();
        cout << "Total recording size is: " << hex << ar.getBufferSize() << dec << endl;
        cout << "Sending data" << endl;

        auto start = chrono::steady_clock::now();
        WAVEHeader hdr = ar.getWaveHeader();
        txMan.setWaveHeader(&hdr);
        txMan.transmitData(1, MSGType::AUDIO, ar.getBuffer(), ar.getBufferSize(), MSGCompression::ADPCM);

        for (;;) {
            if (kbhit()) {
                tx.closePort();
                cout << "exiting" << endl;
                break;
            }

            if (!txMan.tick()) {
                break;
            }
        
        }

        auto end = chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        cout << endl << endl << endl << dec;
        cout << "Transmitted " << ar.getBufferSize() / 1024 << " kB (" 
             << ar.getBufferSize() / 1024 / 4  << " kB after compression) in " 
             << maxMes << " chunks in " << elapsed / 1000.0 << " seconds" << endl
             << "Effective average transmission rate = " 
             << 1000.0 * (double)ar.getBufferSize() / 1024.0 / elapsed 
             << " kB/s" << endl;

        
    } else if (ch == 'r') {
        cout << "Receive side: Openning COM8" << endl;

        COMPort rx = COMPort(baud, CPParity::EVEN, 1);
        rx.openPort("COM8");
        MessageManger rxMan = MessageManger(1, maxMes);
        rxMan.setCOMPort(&rx);
        
        cout << "Port is open. Waiting for input" << endl;


        for (;;) {
            if (kbhit()) {
                ch = _getch();
                // cin >> ch;

                if (ch == 'r') {
                    rxMan.replayAudio();
                } else {
                    rx.closePort();
                    cout << "exiting" << endl;
                    break;
                }

            }

            rxMan.tick();

        }

    } else {

    }
}

