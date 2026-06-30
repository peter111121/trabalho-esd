#ifndef BENCHMARKRESTRICTIONS_H
#define BENCHMARKRESTRICTIONS_H

#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>
#include "Movie.h"
#include "AVLTree.h"
#include "HashTable.h"
#include "LinkedList.h"
#include "Analytics.h"
#include "YearIndex.h"
#include "Corruptor.h"
#include "SlowSort.h"
#include "Restrictions.h"
#include "Timer.h"

// R2: build an AVL but refuse inserts past a cap; report what was dropped.
inline void demoR2_memoryCap(const std::vector<Movie>& movies, size_t cap) {
    AVLTree<std::string, Movie*> avl;
    size_t inserted = 0, rejected = 0;
    for (const Movie& m : movies) {
        if (m.title.empty()) continue;
        if (cap != 0 && avl.size() >= cap) { ++rejected; continue; }
        avl.insert(m.title, const_cast<Movie*>(&m));
        ++inserted;
    }
    std::cout << "  R2 cap=" << cap << ": inserted " << inserted
              << ", rejected " << rejected
              << ", tree height " << avl.treeHeight() << "\n";
}

// R24: time selection sort vs std::sort on the top-rated movies of a range.
inline void demoR24_sort(const YearIndex& idx, int lo, int hi) {
    std::vector<Movie*> a, b;
    idx.forEachInRange(lo, hi, [&](Movie* m){ if (m->criticRating >= 0) a.push_back(m); });
    b = a;   // identical copies so both sort the SAME data

    Timer t;
    t.start();
    std::sort(a.begin(), a.end(), [](Movie* x, Movie* y){ return x->criticRating > y->criticRating; });
    double fast = t.ms();

    t.start();
    selectionSortByRating(b);
    double slow = t.ms();

    std::cout << "  R24 on " << a.size() << " movies: std::sort " << fast
              << " ms vs selection sort " << slow << " ms";
    if (fast > 0) std::cout << "  (" << (slow / fast) << "x slower)";
    std::cout << "\n";
}

// R12: show how a per-op delay dominates an otherwise fast operation.
inline void demoR12_latency(unsigned micros) {
    const int OPS = 200;
    Timer t; t.start();
    volatile long acc = 0;
    for (int i = 0; i < OPS; ++i) {
        acc += i;                                   // trivial "operation"
        if (micros) std::this_thread::sleep_for(std::chrono::microseconds(micros));
    }
    std::cout << "  R12 delay=" << micros << "us/op: " << OPS
              << " ops took " << t.ms() << " ms\n";
}

// R7 (processing): each operation must finish within a time budget; count
// how many searches exceed it. Demonstrates a per-operation computational
// budget constraint across structures of different speeds.
inline void demoR7_budget(const std::vector<Movie>& movies, long long budgetNs) {
    HashTable<Movie*> hash;
    LinkedList<Movie*> list;
    for (const Movie& m : movies) {
        list.pushBack(const_cast<Movie*>(&m));
        if (!m.title.empty()) hash.insert(m.title, const_cast<Movie*>(&m));
    }
    const int OPS = 2000;
    size_t hashViol = 0, listViol = 0;
    Timer t;
    volatile size_t sink = 0;

    for (int i = 0; i < OPS; ++i) {
        const std::string& key = movies[(i * 7) % movies.size()].title;
        t.start();
        if (hash.search(key)) sink += 1;
        if (t.ns() > budgetNs) ++hashViol;

        t.start();
        for (LLNode<Movie*>* nd = list.getHead(); nd; nd = nd->next)
            if (nd->data->title == key) { sink += 1; break; }
        if (t.ns() > budgetNs) ++listViol;
    }
    std::cout << "  R7 budget=" << budgetNs << "ns/op over " << OPS << " ops: "
              << "hash violations=" << hashViol
              << ", list violations=" << listViol << "\n";
    if (sink == SIZE_MAX) std::cout << "";
}

// R16 (data): corrupt a COPY of the data and compare statistics before/after.
inline void demoR16_corruption(const std::vector<Movie>& movies, int lo, int hi) {
    YearIndex cleanIdx;
    for (const Movie& m : movies) cleanIdx.add(m.year, const_cast<Movie*>(&m));
    int skip = 0;
    Stats clean = ratingStats(cleanIdx, lo, hi, false, &skip);

    std::vector<Movie> dmg = movies;                 // copy
    int damaged = corruptData(dmg, 0.20);            // wipe 20%
    YearIndex dmgIdx;
    for (Movie& m : dmg) dmgIdx.add(m.year, &m);
    Stats dirty = ratingStats(dmgIdx, lo, hi, false, &skip);

    std::cout << "  R16 corrupted " << damaged << " records\n";
    std::cout << "    clean   " << lo << "-" << hi << ": n=" << clean.count
              << " mean=" << clean.mean << " sd=" << clean.stddev << "\n";
    std::cout << "    corrupt " << lo << "-" << hi << ": n=" << dirty.count
              << " mean=" << dirty.mean << " sd=" << dirty.stddev << "\n";
}

inline void runAllRestrictions(const std::vector<Movie>& movies) {
    std::cout << "\n=== RESTRICTION SCENARIOS (one per category, before/after) ===\n";

    YearIndex yearIdx;
    for (const Movie& m : movies) yearIdx.add(m.year, const_cast<Movie*>(&m));

    std::cout << "[Category 1 - Memory | R2: cap AVL size]\n";
    demoR2_memoryCap(movies, 0);
    demoR2_memoryCap(movies, 500);

    std::cout << "[Category 2 - Processing | R7: per-operation time budget]\n";
    demoR7_budget(movies, 1000000000LL);   // effectively unlimited (baseline)
    demoR7_budget(movies, 1000);           // 1us budget -> list violates often

    std::cout << "[Category 3 - Latency | R12: injected per-op delay]\n";
    demoR12_latency(0);
    demoR12_latency(500);

    std::cout << "[Category 4 - Data | R16: corrupt 20% of records]\n";
    demoR16_corruption(movies, 2000, 2009);

    std::cout << "[Category 5 - Algorithmic | R24: slow sort vs std::sort]\n";
    demoR24_sort(yearIdx, 2000, 2009);
    demoR24_sort(yearIdx, 1920, 2020);

    std::cout << "(R6 single-core is enforced at launch: run with taskset/affinity "
              << "and compare - documented in the report.)\n";
}

#endif