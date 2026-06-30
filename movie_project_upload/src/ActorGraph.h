#ifndef ACTORGRAPH_H
#define ACTORGRAPH_H

#include <string>
#include <vector>
#include "Movie.h"
#include "HashTable.h"
#include "UnionFind.h"

class ActorGraph {
public:
    ActorGraph() : uf(0) {}

    // Register an actor name, returning its integer id (creating it if new).
    int idOf(const std::string& name) {
        int existing = nameToId.search(name);   // stored ids are >= 1
        if (existing != 0) return existing - 1;  // (we offset by 1; see note)
        int id = (int)names.size();
        names.push_back(name);
        nameToId.insert(name, id + 1);
        uf.ensure(names.size());
        return id;
    }

    // Process one movie: union all of its actors together (they collaborated).
    void addMovie(const Movie* m) {
        if (m->actors.size() < 2) {
            if (m->actors.size() == 1) idOf(m->actors[0]);  // still register solo actors
            return;
        }
        int first = idOf(m->actors[0]);
        for (size_t i = 1; i < m->actors.size(); ++i)
            uf.unite(first, idOf(m->actors[i]));   // link each co-star to the first
    }

    bool connected(const std::string& a, const std::string& b) {
        int ia = nameToId.search(a), ib = nameToId.search(b);
        if (ia == 0 || ib == 0) return false;       // unknown actor
        return uf.connected(ia - 1, ib - 1);
    }

    size_t actorCount()      const { return names.size(); }
    size_t collaborationGroups() const { return uf.componentCount(); }

private:
    std::vector<std::string> names;        // id -> name
    HashTable<int>           nameToId;      // name -> id (offset by +1)
    UnionFind                uf;
};

#endif