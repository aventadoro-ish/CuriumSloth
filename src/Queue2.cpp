#include "Queue2.h"


// Template constructor
template<typename T>
QueueProper<T>::Queue() {
    pHead = pTail = nullptr;  // Initialize the head and tail as null
}

// Template destructor
template<typename T>
QueueProper<T>::~Queue() {
    while (!isQueueEmpty()) {
        free(deQueue());
    }
}

// Initialize queue
template<typename T>
void QueueProper<T>::inItQueue() {
    pHead = pTail = nullptr;
}

// Check if the queue is empty
template<typename T>
bool QueueProper<T>::isQueueEmpty() const {
    return (pHead == nullptr);
}

// Add to queue
template<typename T>
void QueueProper<T>::addToQueue(T* pn) {
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
NodeP<T>* QueueProper<T>::deQueue() {
    if (isQueueEmpty()) {
        std::cerr << "Queue is empty" << std::endl;
        return nullptr;
    }
    NodeP<T>* temp = pHead;
    pHead = pHead->pNext;
    return temp;
}

// Return the head of the queue
template<typename T>
NodeP<T>* QueueProper<T>::returnHead() {
    return pHead;
}

// Count the number of nodes
template<typename T>
int QueueProper<T>::count(NodeP<T>* x) {
    if (x == nullptr) return 0;
    return 1 + count(x->pNext);
}
// Function to visit and print node data
template<typename T>
void visit(NodeP<T>* n) {
    std::cout << "Data: " << n->Data << std::endl;
}


// Recursive delete function
template<typename T>
NodeP<T>* QueueProper<T>::deleteR(NodeP<T>* parent, NodeP<T>* child, T v) {
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
void QueueProper<T>::traverse(NodeP<T>* h, void(*visit)(NodeP<T>*)) {
    if (h == nullptr) return;
    visit(h);
    traverse(h->pNext, visit);
}

// Recursive traverse
template<typename T>
void QueueProper<T>::traverseR(NodeP<T>* h, void(*visit)(NodeP<T>*)) {
    if (h == nullptr) return;
    traverseR(h->pNext, visit);
    visit(h);
}

// Function Prototypes

int testQueue();
