/* Queue.h: Queue header file
 * Date: Oct 2024
 * Author:
 *
 *         Besart Kalezic
 *
 */
#ifndef QUEUE_H
#define QUEUE_H

#include <iostream>
#include <cstdlib>

// Templating the node and item to be of any data type
template<typename T>
struct Node {
    Node* pNext;  // pointer to the next node
    T Data;       // the data is now of template type T
};

template<typename T>
class Queue {
public:
    Queue();                      // constructor
    ~Queue();                     // destructor
    void inItQueue();              // initialize the queue
    bool isQueueEmpty() const;     // check if the queue is empty
    void addToQueue(Node<T>*);     // add an element to the queue
    Node<T>* deQueue();            // dequeue an element

    // Additional queue functions
    Node<T>* returnHead();         // return the head of the queue
    int count(Node<T>* x);         // count the number of nodes
    Node<T>* deleteR(Node<T>* parent, Node<T>* child, T v);  // recursive delete
    void traverse(Node<T>* h, void(*visit)(Node<T>*));       // traverse the queue
    void traverseR(Node<T>* h, void(*visit)(Node<T>*));      // recursive traverse

private:
    Node<T>* pHead;  // head of the queue
    Node<T>* pTail;  // tail of the queue
};

// Template constructor
template<typename T>
Queue<T>::Queue() {
    pHead = pTail = nullptr;  // Initialize the head and tail as null
}

// Template destructor
template<typename T>
Queue<T>::~Queue() {
    while (!isQueueEmpty()) {
        deQueue();
    }
}

// Initialize queue
template<typename T>
void Queue<T>::inItQueue() {
    pHead = pTail = nullptr;
}

// Check if the queue is empty
template<typename T>
bool Queue<T>::isQueueEmpty() const {
    return (pHead == nullptr);
}

// Add to queue
template<typename T>
void Queue<T>::addToQueue(Node<T>* pn) {
    if (isQueueEmpty()) {
        pHead = pTail = pn;
    }
    else {
        pTail->pNext = pn;
        pTail = pn;
    }
    pTail->pNext = nullptr;
}

// Remove from queue
template<typename T>
Node<T>* Queue<T>::deQueue() {
    if (isQueueEmpty()) {
        std::cerr << "Queue is empty" << std::endl;
        return nullptr;
    }
    Node<T>* temp = pHead;
    pHead = pHead->pNext;
    return temp;
}

// Return the head of the queue
template<typename T>
Node<T>* Queue<T>::returnHead() {
    return pHead;
}

// Count the number of nodes
template<typename T>
int Queue<T>::count(Node<T>* x) {
    if (x == nullptr) return 0;
    return 1 + count(x->pNext);
}
// Function to visit and print node data
template<typename T>
void visit(Node<T>* n) {
    std::cout << "Data: " << n->Data << std::endl;
}


// Recursive delete function
template<typename T>
Node<T>* Queue<T>::deleteR(Node<T>* parent, Node<T>* child, T v) {
    if (child == nullptr) return nullptr;
    if (child->Data == v) {
        parent->pNext = child->pNext;
        free(child); // or use delete if you allocate with new
        return deleteR(parent, parent->pNext, v);
    }
    return deleteR(child, child->pNext, v);
}

// Traverse the queue
template<typename T>
void Queue<T>::traverse(Node<T>* h, void(*visit)(Node<T>*)) {
    if (h == nullptr) return;
    visit(h);
    traverse(h->pNext, visit);
}

// Recursive traverse
template<typename T>
void Queue<T>::traverseR(Node<T>* h, void(*visit)(Node<T>*)) {
    if (h == nullptr) return;
    traverseR(h->pNext, visit);
    visit(h);
}

// Function Prototypes

int testQueue();

#endif