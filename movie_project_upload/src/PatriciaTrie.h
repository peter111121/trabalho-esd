#ifndef PATRICIATRIE_H
#define PATRICIATRIE_H

#include <string>
#include <vector>
#include "Movie.h"

class PatriciaTrie {
public:
    static const int ALPHABET = 37;     // same alphabet as MovieTrie

    PatriciaTrie();
    ~PatriciaTrie();
    PatriciaTrie(const PatriciaTrie&) = delete;
    PatriciaTrie& operator=(const PatriciaTrie&) = delete;

    void insert(const std::string& title, Movie* movie);
    std::vector<Movie*> prefixSearch(const std::string& prefix) const;

    size_t nodeCount() const { return nodes; }
    size_t estimatedBytes() const;      // node structs + the label characters

private:
    struct Node {
        std::string label;              // the substring on the edge INTO this node
        Node* children[ALPHABET];       // indexed by first char of the child's label
        std::vector<Movie*> movies;     // titles ending exactly here
        Node();
    };

    Node* root;
    size_t nodes;

    static int indexOf(char c);
    static size_t commonPrefix(const std::string& key, size_t keyStart,
                               const std::string& label);
    void collect(const Node* n, std::vector<Movie*>& out) const;
    size_t sumLabels(const Node* n) const;
    void destroy(Node* n);
};

#endif