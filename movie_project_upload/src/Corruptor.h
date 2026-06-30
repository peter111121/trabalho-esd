#ifndef CORRUPTOR_H
#define CORRUPTOR_H

#include <vector>
#include <random>
#include "Movie.h"

// R16: simulate sensor-style data loss/corruption on `fraction` of records.
// We use a FIXED seed so the corruption is reproducible across runs --
// essential for a fair before/after comparison.
inline int corruptData(std::vector<Movie>& movies, double fraction, unsigned seed = 42) {
    if (fraction <= 0.0) return 0;
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> pick(0.0, 1.0);

    int damaged = 0;
    for (Movie& m : movies) {
        if (pick(rng) < fraction) {
            // Wipe the ratings to the "missing" marker (-1), as if a sensor failed.
            m.criticRating   = -1.0;
            m.audienceRating = -1.0;
            ++damaged;
        }
    }
    return damaged;
}

#endif