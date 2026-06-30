#include "PatriciaTrie.h"
#include "MovieTrie.h"   // reuse the SAME normalize() so the comparison is fair

PatriciaTrie::Node::Node() {
    for (int i = 0; i < ALPHABET; ++i) children[i] = nullptr;
}

PatriciaTrie::PatriciaTrie() : root(new Node()), nodes(1) {}
PatriciaTrie::~PatriciaTrie() { destroy(root); }

void PatriciaTrie::destroy(Node* n) {
    if (!n) return;
    for (int i = 0; i < ALPHABET; ++i) destroy(n->children[i]);
    delete n;
}

int PatriciaTrie::indexOf(char c) {
    if (c >= 'a' && c <= 'z') return c - 'a';
    if (c >= '0' && c <= '9') return 26 + (c - '0');
    if (c == ' ')             return 36;
    return -1;
}

// How many leading characters of key[keyStart..] equal label[0..]?
size_t PatriciaTrie::commonPrefix(const std::string& key, size_t keyStart,
                                  const std::string& label) {
    size_t i = 0;
    while (keyStart + i < key.size() && i < label.size()
           && key[keyStart + i] == label[i])
        ++i;
    return i;
}

void PatriciaTrie::insert(const std::string& title, Movie* movie) {
    std::string norm = MovieTrie::normalize(title);
    if (norm.empty()) return;

    Node* cur = root;
    size_t pos = 0;                          // chars of `norm` matched so far

    while (true) {
        if (pos == norm.size()) {            // whole title consumed at a boundary
            cur->movies.push_back(movie);
            return;
        }

        int i = indexOf(norm[pos]);
        Node* child = cur->children[i];

        if (child == nullptr) {              // no edge starts here:
            Node* leaf = new Node();         //   attach the whole remainder as ONE edge
            leaf->label = norm.substr(pos);
            leaf->movies.push_back(movie);
            cur->children[i] = leaf;
            ++nodes;
            return;
        }

        size_t common = commonPrefix(norm, pos, child->label);

        if (common == child->label.size()) { // matched the entire edge label:
            pos += common;                   //   consume it and descend
            cur = child;
            continue;
        }

        // PARTIAL match -> split the edge at position `common` (the test/team case)
        Node* split = new Node();
        ++nodes;
        split->label = child->label.substr(0, common);    // shared part (e.g. "te")
        child->label = child->label.substr(common);        // old child keeps rest ("st")
        cur->children[i] = split;
        split->children[indexOf(child->label[0])] = child; // re-hang the old child

        size_t rem = pos + common;
        if (rem == norm.size()) {
            split->movies.push_back(movie);  // new title ends exactly at the split
        } else {
            Node* leaf = new Node();         // new title diverges -> new branch ("am")
            ++nodes;
            leaf->label = norm.substr(rem);
            leaf->movies.push_back(movie);
            split->children[indexOf(leaf->label[0])] = leaf;
        }
        return;
    }
}

std::vector<Movie*> PatriciaTrie::prefixSearch(const std::string& prefix) const {
    std::vector<Movie*> out;
    std::string p = MovieTrie::normalize(prefix);

    const Node* cur = root;
    size_t pos = 0;
    while (pos < p.size()) {
        int i = indexOf(p[pos]);
        if (i < 0) return out;
        const Node* child = cur->children[i];
        if (!child) return out;

        const std::string& L = child->label;
        size_t remaining = p.size() - pos;
        size_t cmp = (remaining < L.size()) ? remaining : L.size();
        for (size_t k = 0; k < cmp; ++k)
            if (p[pos + k] != L[k]) return out;   // mismatch within the edge

        if (remaining <= L.size()) {     // prefix ends INSIDE this edge:
            collect(child, out);         //   the whole subtree under child matches
            return out;
        }
        pos += L.size();                 // consumed the whole edge, keep going
        cur = child;
    }
    collect(cur, out);                   // prefix ended exactly at a node boundary
    return out;
}

void PatriciaTrie::collect(const Node* n, std::vector<Movie*>& out) const {
    if (!n) return;
    for (Movie* m : n->movies) out.push_back(m);
    for (int i = 0; i < ALPHABET; ++i)
        if (n->children[i]) collect(n->children[i], out);
}

size_t PatriciaTrie::sumLabels(const Node* n) const {
    if (!n) return 0;
    size_t total = n->label.size();
    for (int i = 0; i < ALPHABET; ++i)
        if (n->children[i]) total += sumLabels(n->children[i]);
    return total;
}

size_t PatriciaTrie::estimatedBytes() const {
    return nodeCount() * sizeof(Node) + sumLabels(root);
}