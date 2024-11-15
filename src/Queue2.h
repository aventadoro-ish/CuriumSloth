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
struct NodeP {
    NodeP* pNext;  // pointer to the next node
    T Data;       // the data is now of template type T
};

template<typename T>
class QueueProper {
public:
    QueueProper();                      // constructor
    ~QueueProper();                     // destructor
    void inItQueue();               // initialize the queue
    bool isQueueEmpty() const;      // check if the queue is empty

    /// @brief copies the element onto the queue
    /// @param element element to be copied
    void addToQueue(T* element);      // add an element to the queue

    /// @brief dequeue an element, pointer is returned - client needs to free 
    /// it when needed
    /// @return pointer to the element
    T* deQueue();

    // Additional queue functions

    /// @brief returns the pointer to the head of the queue.
    /// Must not be freed
    /// @return pointer to the first element
    T* returnHead();         // return the head of the queue

    /// @brief get the number of elements in the queue
    /// @return int number of elements in the queue
    int count();         // count the number of nodes

private:
    
    NodeP<T>* deleteR(NodeP<T>* parent, NodeP<T>* child, T v);  // recursive delete
    void traverse(NodeP<T>* h, void(*visit)(NodeP<T>*));       // traverse the queue
    void traverseR(NodeP<T>* h, void(*visit)(NodeP<T>*));      // recursive traverse

    NodeP<T>* pHead;  // head of the queue
    NodeP<T>* pTail;  // tail of the queue
};

#endif