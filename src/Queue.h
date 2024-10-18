#pragma once
#define QUEUE_H


using namespace std;  // Use the standard namespace to avoid writing 'std::' before standard functions

// Template class declaration and implementation directly in the main file
template <typename T>  // Template allows the class to handle different data types (int, double, etc.)
class QueueGen {
private:
    // Structure to represent a node in the queue
    struct Node {
        T data;  // Stores the value of the node (of type T, which is the template type)
        Node* next;  // Pointer to the next node in the queue

        // Constructor for Node, initializes 'data' with the given value and 'next' to nullptr
        Node(T val) : data(val), next(nullptr) {}
    };

    Node* front;  // Points to the front node in the queue
    Node* rear;   // Points to the rear node in the queue
    int size;     // Tracks the number of elements in the queue

public:
    // Constructor: initializes an empty queue by setting 'front' and 'rear' to nullptr and size to 0
    QueueGen();

    // Destructor: ensures all nodes in the queue are deleted to prevent memory leaks
    ~QueueGen() {
        while (size != 0) {  // While the queue is not empty, call 'dequeue' to remove and delete each node
            dequeue();
        }
    }

    // Enqueue operation: adds a new element to the rear of the queue
    void enqueue(const T& value);// {
 

// Dequeue operation: removes the element from the front of the queue
    void dequeue();// {

 // Peek operation: returns the value of the front element without removing it
    T peek();  
// Returns the current size of the queue; 
    int getSize(); 

};