#include "Queue2.h"
#include <iostream>
#include <cstdlib>
#include <cstring> // for memcpy()


// for compiling Queue<Message>
#include "Message.h"


using namespace std;

template <typename T>
inline QueueProper<T>::QueueProper() {
    head = nullptr;
    tail = nullptr;
}

template <typename T>
QueueProper<T>::~QueueProper() {


}

template <typename T>
T* QueueProper<T>::pop() {
    if (head == nullptr) {
        return nullptr;
    }

    Node2<T>* head_cpy = head;
    head = head->pNext;

    T* res_data = head_cpy->data;


    free(head_cpy);

    return res_data;
}


template <typename T>
void QueueProper<T>::push(T* new_data) {
    if (new_data == nullptr) {
        cerr << "ERROR! Unable to add nullptr to the queue." << endl;
        return;
    }

    if (isEmpty()) {
        head = (Node2<T>*)malloc(sizeof(Node2<T>));
        if (head == nullptr) {
            cerr << "ERROR! malloc() for the first node of a queue failed!" << endl;
            return;
        }
        tail = head;
    } else {
        tail->pNext = (Node2<T>*)malloc(sizeof(Node2<T>));
        if (head == nullptr) {
            cerr << "ERROR! malloc() for the new node of a queue failed!" << endl;
            return;
        }
        tail = tail->pNext;
    }

    // tail->data = (T*)malloc(sizeof(T));
    // tail->pNext = nullptr;
    // if (tail->data == nullptr) {
    //     cerr << "ERROR! malloc() for data of a new node failed!" << endl;
    // }
    // cout << "Queue - copying data from @" << hex << new_data << " to @" << hex << tail->data << endl;
    // memcpy(tail->data, new_data, sizeof(T));

    tail->data = new_data;
    tail->pNext = nullptr;
    return;
}

template <typename T>
T* QueueProper<T>::peek(int n) {
    int count = 0;

    Node2<T>* node = head;

    // 0 -> head; 1 -> head + 1
    while (node != nullptr && count != n) {
        node = node->pNext;
        count++;
    }

    if (node == nullptr) {
        cerr << "ERROR! Trying to peek node that is outside of queue range";
    }


    return nullptr;
}

template <typename T>
bool QueueProper<T>::isEmpty() {
    return head == nullptr; // && tail == nullptr;
}


// for compiling Queue<Message>
template class QueueProper<Message>;
