#include "Queue.h"
#include <iostream>

template <typename T> QueueGen<T>::QueueGen() {
	front = nullptr;
	rear = nullptr;
	size = 0;
}


//template<typename T>
//QueueGen<T>::QueueGen()
//{
//}
//
//template<typename T>
//QueueGen<T>::~QueueGen()
//{
//}



template <typename T> void QueueGen() { front(nullptr), rear(nullptr), size(0) }

// Destructor: ensures all nodes in the queue are deleted to prevent memory leaks
//template <typename T> void ~QueueGen() {
//    while (size != 0) {  // While the queue is not empty, call 'dequeue' to remove and delete each node
//        dequeue();
//    }
//}


template <typename T> void QueueGen<T>::enqueue(const T& value) { // for return type syntac template typename void ....
    Node* newNode = new Node(value);  // Create a new node with the given value
    if (size == 0) {  // If the queue is empty, the new node becomes both front and rear
        front = rear = newNode;
    }
    else {  // Otherwise, add the new node at the rear and update the rear pointer
        rear->next = newNode;
        rear = newNode;
    }
    size++;  // Increase the size of the queue
}


template <typename T> void QueueGen<T>::dequeue() {
    if (size == 0) {  // If the queue is empty, print a message and return
        cout << "Queue is empty!" << endl;
        return;
    }
    Node* temp = front;  // Temporarily store the front node to delete it later
    front = front->next;  // Move the front pointer to the next node
    delete temp;  // Delete the old front node
    size--;  // Decrease the size of the queue
}



template <typename T> T QueueGen<T>::peek()  {
    if (size == 0) {  // If the queue is empty, throw an error
        throw runtime_error("Queue is empty!");
    }
    return front->data;  // Return the data of the front node
}


template <typename T> int QueueGen<T>:: getSize() {
    return size;
}
