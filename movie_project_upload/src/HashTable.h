#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <string>
#include <vector>
#include <iostream>
#include <cstddef>

// Bucket-chain node. Now the stored value is a generic type V.
template <typename V>
struct HashNode {
    std::string key;
    V value;
    HashNode<V>* next;
    HashNode(const std::string& k, const V& v) : key(k), value(v), next(nullptr) {}
};

template <typename V>
class HashTable {
public:
    explicit HashTable(size_t numBuckets = 1024)
        : buckets(numBuckets, nullptr), count(0), collisions(0) {}

    ~HashTable() {                       // free every chain node
        for (HashNode<V>* head : buckets) {
            while (head) {
                HashNode<V>* next = head->next;
                delete head;
                head = next;
            }
        }
    }

    HashTable(const HashTable&) = delete;            // owns raw pointers
    HashTable& operator=(const HashTable&) = delete; // -> forbid copying

    void insert(const std::string& key, const V& value) {
        size_t idx = hash(key);
        for (HashNode<V>* n = buckets[idx]; n; n = n->next)
            if (n->key == key) { n->value = value; return; }   // update existing

        if (buckets[idx] != nullptr) collisions++;
        HashNode<V>* node = new HashNode<V>(key, value);
        node->next = buckets[idx];        // prepend to the chain
        buckets[idx] = node;
        count++;
        if (loadFactor() > 0.75) rehash();
    }

    // Returns the stored value, or V{} when absent.
    // For pointer value types, V{} is nullptr — exactly the old behavior.
    V search(const std::string& key) const {
        size_t idx = hash(key);
        for (HashNode<V>* n = buckets[idx]; n; n = n->next)
            if (n->key == key) return n->value;
        return V{};
    }

    bool remove(const std::string& key) {
        size_t idx = hash(key);
        HashNode<V>* node = buckets[idx];
        HashNode<V>* prev = nullptr;
        while (node) {
            if (node->key == key) {
                if (prev == nullptr) buckets[idx] = node->next;
                else                 prev->next = node->next;
                delete node;
                count--;
                return true;
            }
            prev = node;
            node = node->next;
        }
        return false;
    }

    // Visit every (key, value) pair. We'll use this to free the actor lists.
    template <typename Fn>
    void forEach(Fn fn) const {
        for (HashNode<V>* head : buckets)
            for (HashNode<V>* n = head; n; n = n->next)
                fn(n->key, n->value);
    }

    size_t size() const { return count; }
    double loadFactor() const { return static_cast<double>(count) / buckets.size(); }

    // Structural memory: the bucket array + every chain node + the key chars.
    // (Does not count the heap of the stored value type V.)
    size_t estimatedBytes() const {
        size_t total = buckets.size() * sizeof(HashNode<V>*);
        for (HashNode<V>* head : buckets)
            for (HashNode<V>* n = head; n; n = n->next)
                total += sizeof(HashNode<V>) + n->key.capacity();
        return total;
    }

    void printStats() const {
        size_t used = 0, longest = 0;
        for (HashNode<V>* head : buckets) {
            size_t len = 0;
            for (HashNode<V>* n = head; n; n = n->next) len++;
            if (len > 0) used++;
            if (len > longest) longest = len;
        }
        std::cout << "--- Hash table stats ---\n";
        std::cout << "  entries      : " << count << "\n";
        std::cout << "  buckets      : " << buckets.size() << "\n";
        std::cout << "  used buckets : " << used << "\n";
        std::cout << "  load factor  : " << loadFactor() << "\n";
        std::cout << "  collisions   : " << collisions << "\n";
        std::cout << "  longest chain: " << longest << "\n";
    }

private:
    std::vector<HashNode<V>*> buckets;
    size_t count;
    size_t collisions;

    size_t hashWithSize(const std::string& key, size_t n) const {
        size_t h = 5381;                          // djb2
        for (char c : key)
            h = ((h << 5) + h) + static_cast<unsigned char>(c);  // h*33 + c
        return h % n;
    }
    size_t hash(const std::string& key) const { return hashWithSize(key, buckets.size()); }

    void rehash() {
        std::vector<HashNode<V>*> newBuckets(buckets.size() * 2, nullptr);
        size_t newCollisions = 0;
        for (HashNode<V>* head : buckets) {
            HashNode<V>* node = head;
            while (node) {
                HashNode<V>* next = node->next;   // save before relinking
                size_t idx = hashWithSize(node->key, newBuckets.size());
                if (newBuckets[idx] != nullptr) newCollisions++;
                node->next = newBuckets[idx];
                newBuckets[idx] = node;
                node = next;
            }
        }
        buckets = std::move(newBuckets);
        collisions = newCollisions;
    }
};

#endif