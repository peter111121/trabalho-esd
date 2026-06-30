#ifndef BENCHMARKC_H
#define BENCHMARKC_H

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include "Movie.h"
#include "ActorGraph.h"
#include "CollaborationPath.h"
#include "Timer.h"

inline void runGroupC(std::vector<Movie>& movies) {
    std::cout << "\n=== GROUP C: actor connectivity (Union-Find vs BFS) ===\n";

    Timer t;

    // Build Union-Find graph.
    t.start();
    ActorGraph uf;
    for (Movie& m : movies) uf.addMovie(&m);
    double ufBuild = t.ms();

    // Build BFS adjacency.
    t.start();
    CollaborationPath bfs;
    bfs.build(movies);
    double bfsBuild = t.ms();

    // Collect a set of real actor names to query.
    std::vector<std::string> actors;
    for (const Movie& m : movies)
        for (const std::string& a : m.actors) {
            if (actors.size() >= 400) break;
            actors.push_back(a);
        }

    // Form pairs (every actor vs one a fixed distance away in the list).
    const size_t PAIRS = 200;
    volatile size_t sink = 0;

    // Union-Find: query each pair MANY times (it's so fast we need repetition).
    const size_t UF_REPS = 500;
    t.start();
    for (size_t r = 0; r < UF_REPS; ++r)
        for (size_t i = 0; i + 1 < actors.size() && i < PAIRS; ++i)
            sink += uf.connected(actors[i], actors[i + 1]) ? 1 : 0;
    double ufQuery = (double)t.ns() / (UF_REPS * PAIRS);

    // BFS: each query is expensive, so ONE pass is plenty.
    t.start();
    for (size_t i = 0; i + 1 < actors.size() && i < PAIRS; ++i)
        sink += bfs.shortestPath(actors[i], actors[i + 1]).found ? 1 : 0;
    double bfsQuery = (double)t.ns() / PAIRS;

    if (sink == SIZE_MAX) std::cout << "";

    std::cout << std::fixed << std::setprecision(2);
    std::cout << std::left << std::setw(18) << "metric"
              << std::right << std::setw(16) << "Union-Find"
              << std::setw(16) << "BFS" << "\n";
    std::cout << std::left << std::setw(18) << "build (ms)"
              << std::right << std::setw(16) << ufBuild
              << std::setw(16) << bfsBuild << "\n";
    std::cout << std::left << std::setw(18) << "query (ns)"
              << std::right << std::setprecision(0)
              << std::setw(16) << ufQuery
              << std::setw(16) << bfsQuery << "\n";
    std::cout << "\nNote: Union-Find answers WHETHER (near-constant); BFS also\n"
              << "recovers the PATH but pays graph traversal per query.\n";
}

#endif