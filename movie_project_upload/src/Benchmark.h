#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <random>
#include "Movie.h"
#include "LinkedList.h"
#include "HashTable.h"
#include "AVLTree.h"
#include "Timer.h"

// Pick `howMany` titles spread evenly across the first `n` movies.
// Spreading (not first-n) avoids accidentally favoring one structure's layout.
inline std::vector<std::string> sampleTitles(const std::vector<Movie>& movies,
                                             size_t n, size_t howMany) {
    std::vector<std::string> q;
    if (n == 0) return q;
    size_t stride = (n > howMany) ? n / howMany : 1;
    for (size_t i = 0; i < n && q.size() < howMany; i += stride)
        if (!movies[i].title.empty()) q.push_back(movies[i].title);
    return q;
}

inline void runGroupA(const std::vector<Movie>& movies) {
    // Use a copy of pointers to the first n movies at each size.
    std::vector<size_t> sizes = {1000, 2000, 4000, 8000, movies.size()};
    const size_t QUERIES = 10000;

    std::cout << "\n=== GROUP A: find movie by title (same task, 3 structures) ===\n";
    std::cout << std::left << std::setw(9) << "n"
              << std::right << std::setw(14) << "list build"
              << std::setw(14) << "hash build"
              << std::setw(14) << "avl build"
              << std::setw(14) << "list srch"
              << std::setw(14) << "hash srch"
              << std::setw(14) << "avl srch"
              << "   (build=ms total, srch=ns/op)\n";

    for (size_t n : sizes) {
        if (n > movies.size()) n = movies.size();

        // --- build each structure, timing insertion ---
        LinkedList<Movie*> list;
        HashTable<Movie*>  hash;
        AVLTree<std::string, Movie*> avl;
        Timer t;

        t.start();
        for (size_t i = 0; i < n; ++i) list.pushBack(const_cast<Movie*>(&movies[i]));
        double listBuild = t.ms();

        t.start();
        for (size_t i = 0; i < n; ++i)
            if (!movies[i].title.empty()) hash.insert(movies[i].title, const_cast<Movie*>(&movies[i]));
        double hashBuild = t.ms();

        t.start();
        for (size_t i = 0; i < n; ++i)
            if (!movies[i].title.empty()) avl.insert(movies[i].title, const_cast<Movie*>(&movies[i]));
        double avlBuild = t.ms();

        // --- searches: same query set for all three ---
        std::vector<std::string> queries = sampleTitles(movies, n, QUERIES);
        volatile size_t sink = 0;   // 'volatile' => optimizer must keep the work

        t.start();
        for (size_t r = 0; r < QUERIES; ++r) {
            const std::string& key = queries[r % queries.size()];
            for (LLNode<Movie*>* nd = list.getHead(); nd; nd = nd->next)
                if (nd->data->title == key) { sink += 1; break; }   // linear scan
        }
        double listSearch = (double)t.ns() / QUERIES;

        t.start();
        for (size_t r = 0; r < QUERIES; ++r)
            if (hash.search(queries[r % queries.size()])) sink += 1;
        double hashSearch = (double)t.ns() / QUERIES;

        t.start();
        for (size_t r = 0; r < QUERIES; ++r)
            if (avl.search(queries[r % queries.size()])) sink += 1;
        double avlSearch = (double)t.ns() / QUERIES;

        std::cout << std::left << std::setw(9) << n << std::right << std::fixed
                  << std::setprecision(2)
                  << std::setw(14) << listBuild
                  << std::setw(14) << hashBuild
                  << std::setw(14) << avlBuild
                  << std::setprecision(0)
                  << std::setw(14) << listSearch
                  << std::setw(14) << hashSearch
                  << std::setw(14) << avlSearch << "\n";

        if (sink == SIZE_MAX) std::cout << "";   // touch sink so it can't be removed

        if (n == movies.size()) break;            // already did the full set
    }
}

// The metrics the search/build table doesn't cover: REMOVAL time, COMBINED
// operation latency (insert+search+remove mixed), and the hash COLLISION rate.
inline void runGroupA_extras(const std::vector<Movie>& movies) {
    const size_t N = movies.size();
    std::cout << "\n=== GROUP A (extra metrics): removal, combined latency, collisions ===\n";

    // Build full structures.
    LinkedList<Movie*> list;
    HashTable<Movie*>  hash;
    AVLTree<std::string, Movie*> avl;
    for (size_t i = 0; i < N; ++i) {
        list.pushBack(const_cast<Movie*>(&movies[i]));
        if (!movies[i].title.empty()) {
            hash.insert(movies[i].title, const_cast<Movie*>(&movies[i]));
            avl.insert(movies[i].title, const_cast<Movie*>(&movies[i]));
        }
    }

    // Collision rate (hash-only metric the brief asks for).
    std::cout << "[collision metric]\n";
    hash.printStats();

    // --- MEMORY USE (structural) for each structure, measured before removal ---
    std::cout << "[memory KB]  list=" << (list.estimatedBytes() / 1024)
              << "  hash=" << (hash.estimatedBytes() / 1024)
              << "  avl="  << (avl.estimatedBytes()  / 1024) << "\n";

    Timer t;
    volatile size_t sink = 0;

    // --- AVERAGE QUERY TIME: access RANDOMLY-CHOSEN elements ---
    // Distinct from Group A's search (which sampled keys by stride). Here keys
    // are drawn at random, so this is the average cost to reach a random element.
    {
        std::mt19937 rng(12345);
        std::uniform_int_distribution<size_t> pick(0, N - 1);
        const size_t Q       = 10000;
        std::vector<std::string> rkeys;
        rkeys.reserve(Q);
        for (size_t i = 0; i < Q; ++i) {
            const std::string& tt = movies[pick(rng)].title;
            rkeys.push_back(tt.empty() ? movies[0].title : tt);
        }

        t.start();
        for (const std::string& k : rkeys) if (hash.search(k)) sink += 1;
        double hashQ = (double)t.ns() / Q;

        t.start();
        for (const std::string& k : rkeys) if (avl.search(k)) sink += 1;
        double avlQ = (double)t.ns() / Q;

        // List: random access means scanning for a random element each time.
        t.start();
        for (size_t i = 0; i < 1000; ++i) {            // fewer: each is O(n)
            const std::string& k = rkeys[i];
            for (LLNode<Movie*>* nd = list.getHead(); nd; nd = nd->next)
                if (nd->data->title == k) { sink += 1; break; }
        }
        double listQ = (double)t.ns() / 1000;

        std::cout << "[avg query ns/op] hash=" << hashQ
                  << "  avl=" << avlQ << "  list=" << listQ
                  << " (random element access)\n";
    }

    // --- REMOVAL time: remove a spread-out sample of keys ---
    const size_t REMS = 5000;
    std::vector<std::string> keys = sampleTitles(movies, N, REMS);

    t.start();
    for (const std::string& k : keys) sink += hash.remove(k) ? 1 : 0;
    double hashRem = (double)t.ns() / keys.size();

    t.start();
    for (const std::string& k : keys) sink += avl.remove(k) ? 1 : 0;
    double avlRem = (double)t.ns() / keys.size();

    // Linked-list removal is O(n) per element; time fewer to keep it bounded.
    t.start();
    size_t listRems = 0;
    for (size_t i = 0; i < keys.size() && i < 500; ++i) {
        Movie* m = nullptr;
        for (LLNode<Movie*>* nd = list.getHead(); nd; nd = nd->next)
            if (nd->data->title == keys[i]) { m = nd->data; break; }
        if (m && list.remove(m)) ++listRems;
    }
    double listRem = listRems ? (double)t.ns() / listRems : 0.0;

    std::cout << "[removal ns/op]  hash=" << hashRem
              << "  avl=" << avlRem
              << "  list=" << listRem << " (incl. its own O(n) scan)\n";

    // --- COMBINED latency: mixed insert+search+remove, for ALL THREE ---
    // Definition: average response time over a stream of mixed operations.
    {
        std::vector<std::string> mixKeys = sampleTitles(movies, N, 3000);

        // Hash
        HashTable<Movie*> h2;
        t.start();
        for (size_t i = 0; i < mixKeys.size(); ++i) {
            h2.insert(mixKeys[i], const_cast<Movie*>(&movies[i % N]));
            if (h2.search(mixKeys[i])) sink += 1;
            if (i % 2 == 0) h2.remove(mixKeys[i]);
        }
        double hashMix = (double)t.ns() / (mixKeys.size() * 2.5);

        // AVL
        AVLTree<std::string, Movie*> a2;
        t.start();
        for (size_t i = 0; i < mixKeys.size(); ++i) {
            a2.insert(mixKeys[i], const_cast<Movie*>(&movies[i % N]));
            if (a2.search(mixKeys[i])) sink += 1;
            if (i % 2 == 0) a2.remove(mixKeys[i]);
        }
        double avlMix = (double)t.ns() / (mixKeys.size() * 2.5);

        // List: O(n) search/remove, so use fewer mixed operations.
        LinkedList<Movie*> l2;
        const size_t LMIX = 400;
        t.start();
        for (size_t i = 0; i < LMIX; ++i) {
            Movie* mp = const_cast<Movie*>(&movies[i % N]);
            l2.pushBack(mp);                                   // insert
            for (LLNode<Movie*>* nd = l2.getHead(); nd; nd = nd->next)
                if (nd->data == mp) { sink += 1; break; }      // search
            if (i % 2 == 0) l2.remove(mp);                     // remove half
        }
        double listMix = (double)t.ns() / (LMIX * 2.5);

        std::cout << "[combined latency ns/op] hash=" << hashMix
                  << "  avl=" << avlMix << "  list=" << listMix
                  << " (mixed insert/search/remove)\n";
    }

    if (sink == SIZE_MAX) std::cout << "";
}

#endif