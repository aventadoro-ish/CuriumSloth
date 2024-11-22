#pragma once
#include "PQHeap.h"

// Constructor
template <typename T>
PriorityQueue<T>::PriorityQueue() : head(nullptr) {}

// Destructor
template <typename T>
PriorityQueue<T>::~PriorityQueue() {
    while (head) {
        Node<T>* temp = head;
        head = head->next;
        delete temp;
    }
}

// Check if the queue is empty
template <typename T>
bool PriorityQueue<T>::isEmpty() const {
    return head == nullptr;
}

// Insert an item into the priority queue
template <typename T>
void PriorityQueue<T>::insert(const T& v) {
    Node<T>* newNode = new Node<T>{ v, nullptr };

    if (isEmpty() || head->data.priority < v.priority) {
        newNode->next = head;
        head = newNode;
        return;
    }

    Node<T>* current = head;
    while (current->next && current->next->data.priority >= v.priority) {
        current = current->next;
    }

    newNode->next = current->next;
    current->next = newNode;
}

// Remove and return the highest-priority item
template <typename T>
T PriorityQueue<T>::delMax() {
    if (isEmpty()) {
        throw std::underflow_error("Priority queue is empty");
    }

    Node<T>* temp = head;
    T maxItem = head->data;
    head = head->next;
    delete temp;
    return maxItem;
}