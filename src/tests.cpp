#include "tests.h"

#include <iostream>
#include <string>
#include <cstring>


#include "COMPort.h"
#include "Message.h"
#include "MessageManager.h"
#include "utils.h"



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




void devTesting() {
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

