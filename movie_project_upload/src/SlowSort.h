#ifndef SLOWSORT_H
#define SLOWSORT_H

#include <vector>
#include "Movie.h"

// R24: deliberately O(n^2) selection sort, descending by critic rating.
// Compare its time against std::sort (introsort, O(n log n)) on the SAME data.
inline void selectionSortByRating(std::vector<Movie*>& v) {
    for (size_t i = 0; i + 1 < v.size(); ++i) {
        size_t best = i;
        for (size_t j = i + 1; j < v.size(); ++j)
            if (v[j]->criticRating > v[best]->criticRating)   // find max in rest
                best = j;
        if (best != i) std::swap(v[i], v[best]);
    }
}

#endif