#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <cstddef>   // for size_t

// One node of the list: a value of type T and a pointer to the next node.
template <typename T>
struct LLNode {
    T data;
    LLNode<T>* next;
    explicit LLNode(const T& d) : data(d), next(nullptr) {}
};

template <typename T>
class LinkedList {
public:
    LinkedList() : head(nullptr), tail(nullptr), count(0) {}
    ~LinkedList() { clear(); }

    // We own raw node pointers, so copying could double-free. Forbid it.
    LinkedList(const LinkedList&) = delete;
    LinkedList& operator=(const LinkedList&) = delete;

    // Add to the END of the list. O(1) thanks to the tail pointer.
    void pushBack(const T& value) {
        LLNode<T>* node = new LLNode<T>(value);
        if (tail == nullptr) {          // list was empty
            head = tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
        count++;
    }

    // Remove the FIRST node whose data equals value. Returns true if removed.
    bool remove(const T& value) {
        LLNode<T>* node = head;
        LLNode<T>* prev = nullptr;
        while (node != nullptr) {
            if (node->data == value) {
                if (prev == nullptr) head = node->next;   // removing the head
                else                 prev->next = node->next;
                if (node == tail) tail = prev;            // removing the tail
                delete node;
                count--;
                return true;
            }
            prev = node;
            node = node->next;
        }
        return false;
    }

    LLNode<T>* getHead() const { return head; }   // for traversal
    size_t size() const { return count; }
    bool empty() const { return count == 0; }

    // Structural memory: one node per element (T itself is fixed-size here).
    size_t estimatedBytes() const { return count * sizeof(LLNode<T>); }

    // Free every node. (Frees the NODES only, not the data they point to.)
    void clear() {
        LLNode<T>* node = head;
        while (node != nullptr) {
            LLNode<T>* next = node->next;
            delete node;
            node = next;
        }
        head = tail = nullptr;
        count = 0;
    }

private:
    LLNode<T>* head;
    LLNode<T>* tail;
    size_t count;
};

#endif