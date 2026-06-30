#ifndef RELATIONSHIPQUERIES_H
#define RELATIONSHIPQUERIES_H

#include <string>
#include <vector>
#include "Movie.h"
#include "LinkedList.h"
#include "ActorIndex.h"

// DIRECT collaboration: did A and B share a film?
// Returns the shared movies (empty => never worked together directly).
// Uses the hash-table filmography index — no graph traversal at all.
inline std::vector<Movie*> actedTogether(const ActorIndex& idx,
                                         const std::string& a,
                                         const std::string& b) {
    std::vector<Movie*> shared;
    LinkedList<Movie*>* aFilms = idx.moviesOf(a);
    if (!aFilms) return shared;                 // A unknown
    for (LLNode<Movie*>* n = aFilms->getHead(); n; n = n->next) {
        for (const std::string& castMember : n->data->actors) {
            if (castMember == b) { shared.push_back(n->data); break; }
        }
    }
    return shared;
}

#endif