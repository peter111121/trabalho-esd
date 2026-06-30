#include "MovieTrie.h"

MovieTrie::Node::Node() {
    for (int i = 0; i < ALPHABET; ++i) children[i] = nullptr;
}

MovieTrie::MovieTrie() : root(new Node()), nodes(1) {}   // root counts as 1 node
MovieTrie::~MovieTrie() { destroy(root); }

void MovieTrie::destroy(Node* n) {
    if (!n) return;
    for (int i = 0; i < ALPHABET; ++i) destroy(n->children[i]);  // free children first
    delete n;
}

// Map a NORMALIZED character to a slot 0..36.
int MovieTrie::indexOf(char c) {
    if (c >= 'a' && c <= 'z') return c - 'a';        // 0..25
    if (c >= '0' && c <= '9') return 26 + (c - '0'); // 26..35
    if (c == ' ')             return 36;             // 36
    return -1;                                       // shouldn't happen post-normalize
}

std::string MovieTrie::normalize(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c >= 'A' && c <= 'Z') c = char(c - 'A' + 'a');   // uppercase -> lower
        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == ' ')
            out += c;                                        // keep
        // everything else (punctuation, accents) is dropped
    }
    return out;
}

void MovieTrie::insert(const std::string& title, Movie* movie) {
    std::string norm = normalize(title);
    if (norm.empty()) return;                // title was all punctuation -> skip
    Node* cur = root;
    for (char c : norm) {
        int i = indexOf(c);
        if (cur->children[i] == nullptr) {
            cur->children[i] = new Node();
            ++nodes;
        }
        cur = cur->children[i];
    }
    cur->movies.push_back(movie);            // mark end-of-title, store the movie
}

bool MovieTrie::remove(const std::string& title, Movie* movie) {
    std::string norm = normalize(title);
    if (norm.empty()) return false;
    Node* cur = root;
    for (char c : norm) {                    // walk to the end-of-title node
        int i = indexOf(c);
        if (i < 0 || cur->children[i] == nullptr) return false;
        cur = cur->children[i];
    }
    for (size_t k = 0; k < cur->movies.size(); ++k) {   // unlink the movie
        if (cur->movies[k] == movie) {
            cur->movies.erase(cur->movies.begin() + k);
            return true;
        }
    }
    return false;
}

const MovieTrie::Node* MovieTrie::navigate(const std::string& norm) const {
    const Node* cur = root;
    for (char c : norm) {
        int i = indexOf(c);
        if (i < 0 || cur->children[i] == nullptr) return nullptr;  // prefix not present
        cur = cur->children[i];
    }
    return cur;                              // node where the prefix ends
}

// DFS: gather every movie stored in this subtree.
void MovieTrie::collect(const Node* n, std::vector<Movie*>& out) const {
    if (!n) return;
    for (Movie* m : n->movies) out.push_back(m);
    for (int i = 0; i < ALPHABET; ++i)
        if (n->children[i]) collect(n->children[i], out);
}

std::vector<Movie*> MovieTrie::prefixSearch(const std::string& prefix) const {
    std::vector<Movie*> out;
    const Node* start = navigate(normalize(prefix));
    if (start) collect(start, out);          // collect everything below the prefix
    return out;
}

size_t MovieTrie::estimatedBytes() const {
    return nodeCount() * sizeof(Node);       // structural memory only (see note)
}