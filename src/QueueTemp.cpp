#include <iostream>  
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
    QueueGen() : front(nullptr), rear(nullptr), size(0) {}

    // Destructor: ensures all nodes in the queue are deleted to prevent memory leaks
    ~QueueGen() {
        while (size!=0) {  // While the queue is not empty, call 'dequeue' to remove and delete each node
            dequeue();
        }
    }

    // Checks if the queue is empty by returning true if size is 0, false otherwise
//    bool isEmpty() const {
 //       return size == 0;
 //   }

    // Enqueue operation: adds a new element to the rear of the queue
    void enqueue(const T& value) {
        Node* newNode = new Node(value);  // Create a new node with the given value
        if (size==0) {  // If the queue is empty, the new node becomes both front and rear
            front = rear = newNode;
        }
        else {  // Otherwise, add the new node at the rear and update the rear pointer
            rear->next = newNode;
            rear = newNode;
        }
        size++;  // Increase the size of the queue
    }

    // Dequeue operation: removes the element from the front of the queue
    void dequeue() {
        if (size == 0) {  // If the queue is empty, print a message and return
            cout << "Queue is empty!" << endl;
            return;
        }
        Node* temp = front;  // Temporarily store the front node to delete it later
        front = front->next;  // Move the front pointer to the next node
        delete temp;  // Delete the old front node
        size--;  // Decrease the size of the queue
    }

    // Peek operation: returns the value of the front element without removing it
    T peek() const {
        if (size == 0) {  // If the queue is empty, throw an error
            throw runtime_error("Queue is empty!");
        }
        return front->data;  // Return the data of the front node
    }

    // Returns the current size of the queue
    int getSize() const {
        return size;
    }
};

// Main function to test the queue
int main() {
    QueueGen<int> q;  // Create a queue of integers

    // Enqueue three elements into the queue
    q.enqueue(10);
    q.enqueue(20);
    q.enqueue(30);

    // Print the front element (should print 10)
    cout << "Front element: " << q.peek() << endl;

    // Dequeue the front element and print the new front (should print 20)
    q.dequeue();
    cout << "Front element after dequeue: " << q.peek() << endl;

    // Print the current size of the queue (should print 2)
    cout << "Queue size: " << q.getSize() << endl;

    return 0;  // End of the program
}
