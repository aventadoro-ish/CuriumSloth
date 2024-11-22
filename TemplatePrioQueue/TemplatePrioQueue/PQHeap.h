#pragma once
#include <stdexcept>

// Node structure for the linked list
template <typename T>
struct Node {
    T data;
    Node* next;
};

// Template for the priority queue
template <typename T>
class PriorityQueue {
private:
    Node<T>* head; // Pointer to the head of the linked list

public:
    PriorityQueue();  // Constructor
    ~PriorityQueue(); // Destructor

    bool isEmpty() const;       // Check if the queue is empty
    void insert(const T& v);    // Insert an item into the priority queue
    T delMax();                 // Remove and return the highest-priority item
};
// Include the implementation file
#include "PQHeap.tpp"