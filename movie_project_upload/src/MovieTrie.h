#ifndef MOVIETRIE_H
#define MOVIETRIE_H

#include <string>
#include <vector>
#include "Movie.h"

class MovieTrie {
public:
    static const int ALPHABET = 37;     // a-z (26) + 0-9 (10) + space (1)

    MovieTrie();
    ~MovieTrie();
    MovieTrie(const MovieTrie&) = delete;
    MovieTrie& operator=(const MovieTrie&) = delete;

    void insert(const std::string& title, Movie* movie);

    // Lazy removal: unlink `movie` from its end node. Returns true if found.
    // Empty nodes are left in place (they contribute no search results).
    bool remove(const std::string& title, Movie* movie);

    // Every movie whose normalized title starts with `prefix`.
    std::vector<Movie*> prefixSearch(const std::string& prefix) const;

    // Lowercase + keep only [a-z0-9 space]. Public so baselines can match it.
    static std::string normalize(const std::string& s);

    size_t nodeCount() const { return nodes; }
    size_t estimatedBytes() const;      // memory in the node array structure

private:
    struct Node {
        Node* children[ALPHABET];
        std::vector<Movie*> movies;     // non-empty => one or more titles end here
        Node();
    };

    Node* root;
    size_t nodes;

    static int indexOf(char c);                 // normalized char -> 0..36
    const Node* navigate(const std::string& norm) const;
    void collect(const Node* n, std::vector<Movie*>& out) const;
    void destroy(Node* n);
};

#endif