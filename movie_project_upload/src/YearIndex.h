#ifndef YEARINDEX_H
#define YEARINDEX_H

#include "Movie.h"
#include "LinkedList.h"
#include "AVLTree.h"

class YearIndex {
public:
    YearIndex() = default;

    ~YearIndex() {
        // One list was created per year with `new`; free them all.
        tree.inOrder([](int, LinkedList<Movie*>* list) { delete list; });
    }

    YearIndex(const YearIndex&) = delete;
    YearIndex& operator=(const YearIndex&) = delete;

    void add(int year, Movie* movie) {
        if (year < 0) return;                 // skip unknown years (-1 marker)
        LinkedList<Movie*>* list = tree.search(year);
        if (list == nullptr) {                // first movie of this year
            list = new LinkedList<Movie*>();
            tree.insert(year, list);
        }
        list->pushBack(movie);
    }

    // Call fn(Movie*) for every movie released in [loYear, hiYear], year-ascending.
    template <typename Fn>
    void forEachInRange(int loYear, int hiYear, Fn fn) const {
        tree.rangeQuery(loYear, hiYear, [&](int, LinkedList<Movie*>* list) {
            for (LLNode<Movie*>* n = list->getHead(); n; n = n->next)
                fn(n->data);
        });
    }

    // Remove `movie` from its year bucket. Returns true if found.
    bool removeMovie(int year, Movie* movie) {
        if (year < 0) return false;
        LinkedList<Movie*>* list = tree.search(year);
        if (list == nullptr) return false;
        return list->remove(movie);
    }

    int distinctYears() const { return (int)tree.size(); }
    int height() const { return tree.treeHeight(); }

private:
    AVLTree<int, LinkedList<Movie*>*> tree;   // year -> that year's movies
};

#endif