/* Queue.h: Queue header file
 * Date: Oct 2024
 * Author: Matvey Regentov
 *
 */
#pragma once


// Templating the node and item to be of any data type
template<typename T>
struct Node2 {
    Node2* pNext;  // pointer to the next node
    T* data;       // the data is now of template type T

    Node2() {
        pNext = nullptr;
        data = nullptr;
    };

};


template<typename T>
class QueueProper {
private:
    Node2<T>* head;
    Node2<T>* tail;

public:
    QueueProper();
    ~QueueProper();


    /// @brief Retrieves the data from the queue. After pop(), this 
    /// class does not manage the deletion of returned data
    /// @return nullptr is queue is empty, pointer to data object otherwise
    T* pop();


    /// @brief Adds the data T to the queue 
    /// (does not copy it, don't free new_data w/o popping it)
    /// @param new_data data to copy into the queue
    void push(T* new_data);

    // untested method!!!
    T* peek(int n);

    bool isEmpty();


};

