
#include "PQHeap.h"
#include <iostream>


struct Item {
    int priority;
    char itemText[30];
    int num;
};
int main() {
    PriorityQueue<Item> pq;

    // Example usage
    Item item1 = { 10, "Item1", 1 };
    Item item2 = { 20, "Item2", 2 };
    Item item3 = { 15, "Item3", 3 };

    pq.insert(item1);
    pq.insert(item2);
    pq.insert(item3);

    while (!pq.isEmpty()) {
        Item maxItem = pq.delMax();
        std::cout << "Priority: " << maxItem.priority
            << ", Text: " << maxItem.itemText << std::endl;
    }

    return 0;
}