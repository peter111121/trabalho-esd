#ifndef BENCHMARKB_H
#define BENCHMARKB_H

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include "Movie.h"
#include "MovieTrie.h"
#include "PatriciaTrie.h"
#include "Timer.h"

inline void runGroupB(const std::vector<Movie>& movies) {
    std::cout << "\n=== GROUP B: prefix search (same task, Trie vs Patricia) ===\n";

    // Build both, timing insertion.
    MovieTrie    plain;
    PatriciaTrie radix;
    Timer t;

    t.start();
    for (const Movie& m : movies)
        if (!m.title.empty()) plain.insert(m.title, const_cast<Movie*>(&m));
    double plainBuild = t.ms();

    t.start();
    for (const Movie& m : movies)
        if (!m.title.empty()) radix.insert(m.title, const_cast<Movie*>(&m));
    double radixBuild = t.ms();

    // A basket of prefixes: rare, medium, and common selectivity.
    std::vector<std::string> prefixes = {
        "star", "the ", "love", "war", "a", "zzzz", "spider", "dark", "king", "b"
    };
    const size_t REPS = 2000;

    volatile size_t sink = 0;

    t.start();
    for (size_t r = 0; r < REPS; ++r)
        for (const std::string& p : prefixes)
            sink += plain.prefixSearch(p).size();   // .size() forces the work to be used
    double plainSearch = (double)t.ns() / (REPS * prefixes.size());

    t.start();
    for (size_t r = 0; r < REPS; ++r)
        for (const std::string& p : prefixes)
            sink += radix.prefixSearch(p).size();
    double radixSearch = (double)t.ns() / (REPS * prefixes.size());

    if (sink == SIZE_MAX) std::cout << "";   // keep sink alive

    std::cout << std::fixed << std::setprecision(2);
    std::cout << std::left << std::setw(16) << "metric"
              << std::right << std::setw(16) << "plain Trie"
              << std::setw(16) << "Patricia" << "\n";
    std::cout << std::left << std::setw(16) << "build (ms)"
              << std::right << std::setw(16) << plainBuild
              << std::setw(16) << radixBuild << "\n";
    std::cout << std::left << std::setw(16) << "nodes"
              << std::right << std::setw(16) << plain.nodeCount()
              << std::setw(16) << radix.nodeCount() << "\n";
    std::cout << std::left << std::setw(16) << "memory (KB)"
              << std::right << std::setw(16) << (plain.estimatedBytes() / 1024)
              << std::setw(16) << (radix.estimatedBytes() / 1024) << "\n";
    std::cout << std::left << std::setw(16) << "search (ns/q)"
              << std::right << std::setprecision(0)
              << std::setw(16) << plainSearch
              << std::setw(16) << radixSearch << "\n";
}

#endif