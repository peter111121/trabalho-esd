#ifndef UNIONFIND_H
#define UNIONFIND_H

#include <vector>
#include <cstddef>

class UnionFind {
public:
    explicit UnionFind(size_t n = 0) { reset(n); }

    void reset(size_t n) {
        parent.resize(n);
        rank.assign(n, 0);
        components = n;
        for (size_t i = 0; i < n; ++i) parent[i] = i;   // each element its own root
    }

    // Grow to hold at least n elements; new ones start as their own group.
    void ensure(size_t n) {
        size_t old = parent.size();
        if (n <= old) return;
        parent.resize(n);
        rank.resize(n, 0);
        for (size_t i = old; i < n; ++i) parent[i] = i;
        components += (n - old);
    }

    // FIND with path compression: returns the root, and flattens the path to it.
    int find(int x) {
        int root = x;
        while (parent[root] != root)      // walk up to the root
            root = parent[root];
        while (parent[x] != root) {       // second pass: point everyone at root
            int next = parent[x];
            parent[x] = root;
            x = next;
        }
        return root;
    }

    // UNION by rank: merge the groups of a and b. Returns false if already together.
    bool unite(int a, int b) {
        int ra = find(a), rb = find(b);
        if (ra == rb) return false;       // already in the same group

        if (rank[ra] < rank[rb]) {        // attach shorter tree under taller
            parent[ra] = rb;
        } else if (rank[ra] > rank[rb]) {
            parent[rb] = ra;
        } else {                          // equal height: pick one, bump its rank
            parent[rb] = ra;
            rank[ra]++;
        }
        components--;                     // two groups became one
        return true;
    }

    bool connected(int a, int b) { return find(a) == find(b); }
    size_t componentCount() const { return components; }
    size_t size() const { return parent.size(); }

private:
    std::vector<int> parent;
    std::vector<int> rank;
    size_t components = 0;
};

#endif