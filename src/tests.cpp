#include "tests.h"

#include <iostream>
#include <string>
#include <cstring>


#include "COMPort.h"
#include "Message.h"
#include "MessageManager.h"
#include "utils.h"
#include <conio.h>



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