#ifndef ACTORINDEX_H
#define ACTORINDEX_H

#include <string>
#include "Movie.h"
#include "LinkedList.h"
#include "HashTable.h"

class ActorIndex {
public:
    ActorIndex() = default;

    ~ActorIndex() {
        // We created one LinkedList per actor with `new`. Free them all here,
        // before the inner table is destroyed.
        table.forEach([](const std::string&, LinkedList<Movie*>* list) {
            delete list;
        });
    }

    ActorIndex(const ActorIndex&) = delete;
    ActorIndex& operator=(const ActorIndex&) = delete;

    // Record that `movie` features `actorName`.
    void add(const std::string& actorName, Movie* movie) {
        LinkedList<Movie*>* list = table.search(actorName);
        if (list == nullptr) {                 // first time seeing this actor
            list = new LinkedList<Movie*>();
            table.insert(actorName, list);
        }
        list->pushBack(movie);                 // append to the filmography
    }

    // This actor's filmography, or nullptr if unknown. O(1) average.
    LinkedList<Movie*>* moviesOf(const std::string& actorName) const {
        return table.search(actorName);
    }

    size_t actorCount() const { return table.size(); }

private:
    HashTable<LinkedList<Movie*>*> table;   // actor name -> filmography list
};

#endif