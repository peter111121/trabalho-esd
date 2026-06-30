#ifndef AVLTREE_H
#define AVLTREE_H

#include <cstddef>
#include <type_traits>

template <typename Key, typename Value>
struct AVLNode {
    Key   key;
    Value value;
    AVLNode* left;
    AVLNode* right;
    int   height;                       // height of this subtree (leaf = 1)
    AVLNode(const Key& k, const Value& v)
        : key(k), value(v), left(nullptr), right(nullptr), height(1) {}
};

template <typename Key, typename Value>
class AVLTree {
public:
    AVLTree() : root(nullptr), count(0) {}
    ~AVLTree() { destroy(root); }

    AVLTree(const AVLTree&) = delete;            // owns raw nodes -> no copy
    AVLTree& operator=(const AVLTree&) = delete;

    void insert(const Key& k, const Value& v) { root = insertNode(root, k, v); }

    Value search(const Key& k) const {           // iterative: O(log n)
        AVLNode<Key,Value>* n = root;
        while (n) {
            if (k == n->key) return n->value;
            n = (k < n->key) ? n->left : n->right;
        }
        return Value{};                          // not found (nullptr for pointers)
    }

    bool remove(const Key& k) {
        bool removed = false;
        root = removeNode(root, k, removed);
        if (removed) count--;
        return removed;
    }

    size_t size() const { return count; }
    int treeHeight() const { return height(root); }

    // Structural memory: every node + (for string keys) the key characters.
    size_t estimatedBytes() const {
        size_t total = 0;
        countBytes(root, total);
        return total;
    }

    // Visit every entry in ascending key order. Fn is called as fn(key, value).
    template <typename Fn>
    void inOrder(Fn fn) const { inOrderRec(root, fn); }

    // Visit every entry with lo <= key <= hi, ascending. Powers range queries.
    template <typename Fn>
    void rangeQuery(const Key& lo, const Key& hi, Fn fn) const {
        rangeRec(root, lo, hi, fn);
    }

private:
    AVLNode<Key,Value>* root;
    size_t count;

    // Sum node sizes; for std::string keys, also count their character storage.
    void countBytes(AVLNode<Key,Value>* n, size_t& total) const {
        if (!n) return;
        total += sizeof(AVLNode<Key,Value>);
        if constexpr (std::is_same_v<Key, std::string>) total += n->key.capacity();
        countBytes(n->left, total);
        countBytes(n->right, total);
    }

    int height(AVLNode<Key,Value>* n) const { return n ? n->height : 0; }

    int balanceFactor(AVLNode<Key,Value>* n) const {
        return n ? height(n->left) - height(n->right) : 0;
    }

    void updateHeight(AVLNode<Key,Value>* n) {
        int hl = height(n->left), hr = height(n->right);
        n->height = (hl > hr ? hl : hr) + 1;
    }

    AVLNode<Key,Value>* rightRotate(AVLNode<Key,Value>* y) {
        AVLNode<Key,Value>* x = y->left;
        AVLNode<Key,Value>* t = x->right;
        x->right = y;                 // y becomes x's right child
        y->left  = t;                 // t (x's old right) becomes y's left
        updateHeight(y);              // update the LOWER node first
        updateHeight(x);
        return x;                     // x is the new subtree root
    }

    AVLNode<Key,Value>* leftRotate(AVLNode<Key,Value>* x) {
        AVLNode<Key,Value>* y = x->right;
        AVLNode<Key,Value>* t = y->left;
        y->left  = x;
        x->right = t;
        updateHeight(x);
        updateHeight(y);
        return y;
    }

    // Restore balance at node n and return the (possibly new) subtree root.
    AVLNode<Key,Value>* rebalance(AVLNode<Key,Value>* n) {
        updateHeight(n);
        int bf = balanceFactor(n);

        if (bf > 1) {                              // left-heavy
            if (balanceFactor(n->left) < 0)        //   Left-Right case
                n->left = leftRotate(n->left);
            return rightRotate(n);                 //   Left-Left case
        }
        if (bf < -1) {                             // right-heavy
            if (balanceFactor(n->right) > 0)       //   Right-Left case
                n->right = rightRotate(n->right);
            return leftRotate(n);                  //   Right-Right case
        }
        return n;                                  // already balanced
    }

    AVLNode<Key,Value>* insertNode(AVLNode<Key,Value>* n, const Key& k, const Value& v) {
        if (!n) { count++; return new AVLNode<Key,Value>(k, v); }
        if (k < n->key)        n->left  = insertNode(n->left, k, v);
        else if (n->key < k)   n->right = insertNode(n->right, k, v);
        else { n->value = v; return n; }           // duplicate key -> update value
        return rebalance(n);
    }

    AVLNode<Key,Value>* minNode(AVLNode<Key,Value>* n) const {
        while (n->left) n = n->left;
        return n;
    }

    AVLNode<Key,Value>* removeNode(AVLNode<Key,Value>* n, const Key& k, bool& removed) {
        if (!n) return nullptr;
        if (k < n->key)        n->left  = removeNode(n->left, k, removed);
        else if (n->key < k)   n->right = removeNode(n->right, k, removed);
        else {
            removed = true;
            if (!n->left || !n->right) {           // 0 or 1 child
                AVLNode<Key,Value>* child = n->left ? n->left : n->right;
                delete n;
                return child;                      // child may be nullptr
            }
            // 2 children: replace with in-order successor, then delete it below
            AVLNode<Key,Value>* succ = minNode(n->right);
            n->key   = succ->key;
            n->value = succ->value;
            bool dummy = false;
            n->right = removeNode(n->right, succ->key, dummy);
        }
        return rebalance(n);                       // rebalance on the way up
    }

    void destroy(AVLNode<Key,Value>* n) {
        if (!n) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }

    template <typename Fn>
    void inOrderRec(AVLNode<Key,Value>* n, Fn& fn) const {
        if (!n) return;
        inOrderRec(n->left, fn);
        fn(n->key, n->value);
        inOrderRec(n->right, fn);
    }

    template <typename Fn>
    void rangeRec(AVLNode<Key,Value>* n, const Key& lo, const Key& hi, Fn& fn) const {
        if (!n) return;
        if (lo < n->key) rangeRec(n->left, lo, hi, fn);          // left only if it could hold keys >= lo
        if (!(n->key < lo) && !(hi < n->key)) fn(n->key, n->value);  // lo <= key <= hi
        if (n->key < hi) rangeRec(n->right, lo, hi, fn);         // right only if it could hold keys <= hi
    }
};
#endif