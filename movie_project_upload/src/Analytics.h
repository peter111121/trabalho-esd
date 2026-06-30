#ifndef ANALYTICS_H
#define ANALYTICS_H

#include <vector>
#include <algorithm>
#include <cmath>
#include "Movie.h"
#include "YearIndex.h"

struct Stats {
    int    count  = 0;     // movies with a valid rating
    double mean   = 0.0;
    double stddev = 0.0;
};

// Mean and (population) standard deviation over a chosen rating field, in
// [lo, hi]. `useAudience` selects which score to summarize. Missing values
// (-1) are skipped and counted separately via `skipped`.
inline Stats ratingStats(const YearIndex& idx, int loYear, int hiYear,
                         bool useAudience = false, int* skipped = nullptr) {
    std::vector<double> vals;
    int miss = 0;
    idx.forEachInRange(loYear, hiYear, [&](Movie* m) {
        double r = useAudience ? m->audienceRating : m->criticRating;
        if (r >= 0.0) vals.push_back(r);
        else ++miss;
    });
    if (skipped) *skipped = miss;

    Stats s;
    s.count = (int)vals.size();
    if (s.count == 0) return s;

    double sum = 0.0;
    for (double v : vals) sum += v;
    s.mean = sum / s.count;                       // first pass: mean

    double sq = 0.0;
    for (double v : vals) { double d = v - s.mean; sq += d * d; }
    s.stddev = std::sqrt(sq / s.count);           // second pass: spread
    return s;
}

// Top-N movies in [lo, hi], highest first. By default ranks by the AVERAGE of
// the critic and audience scores (both must be present). `minCritics` filters
// out movies reviewed by too few critics (the "100% from 3 reviewers" problem).
inline std::vector<Movie*> topByRating(const YearIndex& idx, int loYear, int hiYear,
                                       int n, int minCritics = 0) {
    std::vector<Movie*> v;
    idx.forEachInRange(loYear, hiYear, [&](Movie* m) {
        if (m->criticRating < 0.0 || m->audienceRating < 0.0) return;    // need both
        if (m->criticCount >= 0 && m->criticCount < minCritics) return;  // too few reviews
        v.push_back(m);
    });
    std::sort(v.begin(), v.end(), [](Movie* a, Movie* b) {
        double avgA = (a->criticRating + a->audienceRating) / 2.0;
        double avgB = (b->criticRating + b->audienceRating) / 2.0;
        return avgA > avgB;                                              // descending
    });
    if ((int)v.size() > n) v.resize(n);
    return v;
}

// Convenience: the combined (average) score of a movie. -1 if either is missing.
inline double combinedScore(const Movie* m) {
    if (m->criticRating < 0.0 || m->audienceRating < 0.0) return -1.0;
    return (m->criticRating + m->audienceRating) / 2.0;
}

#endif