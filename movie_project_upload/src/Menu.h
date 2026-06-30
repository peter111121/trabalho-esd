#ifndef MENU_H
#define MENU_H

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <limits>
#include "Movie.h"
#include "HashTable.h"
#include "LinkedList.h"
#include "AVLTree.h"
#include "ActorIndex.h"
#include "YearIndex.h"
#include "MovieTrie.h"
#include "ActorGraph.h"
#include "CollaborationPath.h"
#include "Analytics.h"
#include "RelationshipQueries.h"
#include "Benchmark.h"
#include "BenchmarkB.h"
#include "BenchmarkC.h"
#include "BenchmarkRestrictions.h"

class Menu {
public:
    explicit Menu(std::vector<Movie>& movieList) : movies(movieList) { buildAll(); }

    // Free movies that were added at runtime (heap-owned). Originals belong to
    // the `movies` vector and are NOT freed here.
    ~Menu() { for (Movie* m : ownedMovies) delete m; }

    Menu(const Menu&) = delete;
    Menu& operator=(const Menu&) = delete;

    void run() {
        while (true) {
            printMenu();
            int choice = readInt("Choose an option: ");
            std::cout << "\n";
            switch (choice) {
                case 1: findByTitle();      break;
                case 2: prefixSearch();     break;
                case 3: actorFilmography(); break;
                case 4: yearRangeStats();   break;
                case 5: topMovies();        break;
                case 6: relationship();     break;
                case 7: addMovie();         break;
                case 8: removeMovie();      break;
                case 9: runBenchmarks();    break;
                case 10: runRestrictions(); break;
                case 0: std::cout << "Goodbye.\n"; return;
                default: std::cout << "Unknown option.\n";
            }
            std::cout << "\n";
        }
    }

private:
    std::vector<Movie>&          movies;        // original data (never appended to)
    std::vector<Movie*>          ownedMovies;   // runtime-added movies (heap-owned)
    HashTable<Movie*>            titleHash;
    AVLTree<std::string, Movie*> titleAVL;
    MovieTrie                    trie;
    ActorIndex                   actorIdx;
    YearIndex                    yearIdx;
    ActorGraph                   uf;
    CollaborationPath            bfs;

    void buildAll() {
        std::cout << "Building indices over " << movies.size() << " movies...\n";
        for (Movie& m : movies) indexMovie(&m);
        bfs.build(movies);
        std::cout << "Ready.\n\n";
    }

    // Insert one movie into every searchable index (shared by build and add).
    void indexMovie(Movie* m) {
        if (!m->title.empty()) {
            titleHash.insert(m->title, m);
            titleAVL.insert(m->title, m);
            trie.insert(m->title, m);
        }
        for (const std::string& a : m->actors) actorIdx.add(a, m);
        yearIdx.add(m->year, m);
        uf.addMovie(m);                 // Union-Find updates incrementally
    }

    void printMenu() {
        std::cout << "============ MOVIE DATA SYSTEM ============\n";
        std::cout << " 1. Find a movie by exact title   (hash table)\n";
        std::cout << " 2. Search titles by prefix        (Trie)\n";
        std::cout << " 3. An actor's filmography         (hash + linked list)\n";
        std::cout << " 4. Rating stats for a year range  (year AVL)\n";
        std::cout << " 5. Top movies in a year range     (year AVL + sort)\n";
        std::cout << " 6. Relationship between 2 actors  (3 structures)\n";
        std::cout << " 7. Add a movie                    (insert)\n";
        std::cout << " 8. Remove a movie                 (remove)\n";
        std::cout << " 9. Run performance benchmarks     (Groups A/B/C)\n";
        std::cout << "10. Run restriction scenarios      (5 categories)\n";
        std::cout << " 0. Exit\n";
        std::cout << "===========================================\n";
    }

    // --- input helpers ---
    int readInt(const std::string& prompt) {
        std::cout << prompt;
        int v;
        if (!(std::cin >> v)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return -1;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return v;
    }
    double readDouble(const std::string& prompt) {
        std::cout << prompt;
        double v;
        if (!(std::cin >> v)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return -1.0;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return v;
    }
    std::string readLine(const std::string& prompt) {
        std::cout << prompt;
        std::string s;
        std::getline(std::cin, s);
        return s;
    }

    // --- features ---
    void findByTitle() {
        std::string t = readLine("Exact title: ");
        Movie* m = titleHash.search(t);
        if (m) std::cout << m->title << " (" << m->year << ") - critic "
                         << m->criticRating << ", audience " << m->audienceRating << "\n";
        else   std::cout << "Not found. (Try option 2 for partial titles.)\n";
    }

    void prefixSearch() {
        std::string p = readLine("Title starts with: ");
        std::vector<Movie*> hits = trie.prefixSearch(p);
        std::cout << hits.size() << " match(es):\n";
        int shown = 0;
        for (Movie* m : hits) {
            if (shown++ >= 15) { std::cout << "  ...(" << hits.size() - 15 << " more)\n"; break; }
            std::cout << "  - " << m->title << " (" << m->year << ")\n";
        }
    }

    void actorFilmography() {
        std::string a = readLine("Actor name: ");
        LinkedList<Movie*>* films = actorIdx.moviesOf(a);
        if (!films) { std::cout << "Actor not found.\n"; return; }
        std::cout << a << " appears in " << films->size() << " film(s):\n";
        std::cout << "  (role data is not available in this dataset)\n";
        for (LLNode<Movie*>* n = films->getHead(); n; n = n->next) {
            Movie* m = n->data;
            std::cout << "  - " << m->title << " (" << m->year << ")"
                      << "  critic=" << m->criticRating
                      << "  audience=" << m->audienceRating << "\n";
        }
    }

    void yearRangeStats() {
        int lo = readInt("From year: ");
        int hi = readInt("To year: ");
        int skC = 0, skA = 0;
        Stats c = ratingStats(yearIdx, lo, hi, false, &skC);   // critic
        Stats a = ratingStats(yearIdx, lo, hi, true,  &skA);   // audience
        std::cout << "Ratings " << lo << "-" << hi << ":\n";
        std::cout << "  CRITIC   : n=" << c.count << " (" << skC << " missing)"
                  << "  mean=" << c.mean << "  sd=" << c.stddev << "\n";
        std::cout << "  AUDIENCE : n=" << a.count << " (" << skA << " missing)"
                  << "  mean=" << a.mean << "  sd=" << a.stddev << "\n";
    }

    void topMovies() {
        int lo  = readInt("From year: ");
        int hi  = readInt("To year: ");
        int n   = readInt("How many: ");
        int min = readInt("Min critic reviews (e.g. 20, or 0 for none): ");
        std::cout << "\nRank  Critic  Audience  Avg   Title\n";
        int rank = 1;
        for (Movie* m : topByRating(yearIdx, lo, hi, n, min)) {
            std::cout << "  " << rank++ << "     " << m->criticRating
                      << "      " << m->audienceRating
                      << "     " << combinedScore(m)
                      << "   " << m->title << " (" << m->year << ")\n";
        }
        std::cout << "(ranked by the average of critic and audience scores; "
                  << "at least " << min << " critic reviews)\n";
    }

    void relationship() {
        std::string a = readLine("First actor: ");
        std::string b = readLine("Second actor: ");

        std::vector<Movie*> shared = actedTogether(actorIdx, a, b);
        std::cout << "[1] Acted together? ";
        if (shared.empty()) std::cout << "no direct film\n";
        else {
            std::cout << "yes, in " << shared.size() << " film(s):\n";
            for (Movie* m : shared) std::cout << "      - " << m->title << "\n";
        }

        std::cout << "[2] Same network?   "
                  << (uf.connected(a, b) ? "CONNECTED" : "not connected") << "\n";

        std::cout << "[3] Shortest chain: ";
        CollaborationPath::Result r = bfs.shortestPath(a, b);
        if (!r.found) std::cout << "no path\n";
        else {
            std::cout << r.degrees() << " degree(s)\n";
            std::string cur = r.source;
            for (const auto& step : r.steps) {
                std::cout << "      " << cur << " --[ " << step.movie->title
                          << " ]--> " << step.actor << "\n";
                cur = step.actor;
            }
        }
    }

    // --- INSERT: build a movie at runtime and index it ---
    void addMovie() {
        Movie* m = new Movie();
        m->title = readLine("Title: ");
        if (m->title.empty()) { std::cout << "Title required. Cancelled.\n"; delete m; return; }
        if (titleHash.search(m->title)) {
            std::cout << "A movie with that exact title already exists. Cancelled.\n";
            delete m; return;
        }
        m->year          = readInt("Year (or -1 if unknown): ");
        m->criticRating  = readDouble("Critic rating 0-100 (or -1): ");
        m->audienceRating = readDouble("Audience rating 0-100 (or -1): ");
        std::string actorsLine = readLine("Actors (comma-separated): ");
        std::stringstream ss(actorsLine);
        std::string a;
        while (std::getline(ss, a, ',')) {
            size_t s0 = a.find_first_not_of(" \t");
            size_t s1 = a.find_last_not_of(" \t");
            if (s0 != std::string::npos) m->actors.push_back(a.substr(s0, s1 - s0 + 1));
        }

        ownedMovies.push_back(m);   // we own this heap object
        indexMovie(m);
        std::cout << "Added \"" << m->title << "\". Now searchable in all indices.\n";
    }

    // --- REMOVE: unlink a movie from every searchable index ---
    void removeMovie() {
        std::string t = readLine("Exact title to remove: ");
        Movie* m = titleHash.search(t);
        if (!m) { std::cout << "Not found.\n"; return; }

        titleHash.remove(t);
        titleAVL.remove(t);
        trie.remove(t, m);
        for (const std::string& a : m->actors) {
            LinkedList<Movie*>* films = actorIdx.moviesOf(a);
            if (films) films->remove(m);
        }
        yearIdx.removeMovie(m->year, m);
        // Note: Union-Find and the BFS graph are not de-linked (Union-Find does
        // not support removal by design; the path graph is rebuilt only at startup).

        std::cout << "Removed \"" << t << "\" from the searchable indices.\n";

        // If it was a runtime-added movie, free it now.
        for (size_t i = 0; i < ownedMovies.size(); ++i) {
            if (ownedMovies[i] == m) {
                delete m;
                ownedMovies.erase(ownedMovies.begin() + i);
                break;
            }
        }
    }

    void runBenchmarks() {
        runGroupA(movies);
        runGroupA_extras(movies);
        runGroupB(movies);
        runGroupC(movies);
    }

    void runRestrictions() {
        runAllRestrictions(movies);
    }
};

#endif
