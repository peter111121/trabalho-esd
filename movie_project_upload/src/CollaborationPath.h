#ifndef COLLABORATIONPATH_H
#define COLLABORATIONPATH_H

#include <string>
#include <vector>
#include <queue>
#include "Movie.h"
#include "HashTable.h"

class CollaborationPath {
public:
    // One link in the chain: actor `to` joined via film `movie`.
    struct Step { Movie* movie; std::string actor; };
    struct Result {
        bool found = false;
        std::string source;
        std::vector<Step> steps;       // source --movie--> steps[0].actor --...
        int degrees() const { return (int)steps.size(); }   // number of films traversed
    };

    // Build the actor->films map once from all movies.
    void build(std::vector<Movie>& movies) {
        for (Movie& m : movies)
            for (const std::string& a : m.actors) {
                int id = idOf(a);
                films[id].push_back(&m);
            }
    }

    Result shortestPath(const std::string& from, const std::string& to) {
        Result r; r.source = from;
        int s = lookup(from), t = lookup(to);
        if (s < 0 || t < 0 || s == t) return r;   // unknown actor or same actor

        size_t N = names.size();
        std::vector<int>    prevActor(N, -1);
        std::vector<Movie*> prevMovie(N, nullptr);
        std::vector<char>   visited(N, 0);

        std::queue<int> q;
        q.push(s); visited[s] = 1;

        while (!q.empty()) {
            int u = q.front(); q.pop();
            if (u == t) { reconstruct(r, prevActor, prevMovie, s, t); return r; }

            // u's neighbors: every co-star in every film u appeared in.
            for (Movie* film : films[u]) {
                for (const std::string& coName : film->actors) {
                    int v = lookup(coName);
                    if (v < 0 || visited[v]) continue;
                    visited[v]    = 1;
                    prevActor[v]  = u;
                    prevMovie[v]  = film;     // the film that links u -> v
                    if (v == t) { reconstruct(r, prevActor, prevMovie, s, t); return r; }
                    q.push(v);
                }
            }
        }
        return r;   // not connected: r.found stays false
    }

    size_t actorCount() const { return names.size(); }

private:
    std::vector<std::string>          names;   // id -> name
    std::vector<std::vector<Movie*>>  films;   // id -> that actor's movies
    HashTable<int>                    nameToId; // name -> id+1 (0 means absent)

    int idOf(const std::string& name) {        // get-or-create
        int e = nameToId.search(name);
        if (e != 0) return e - 1;
        int id = (int)names.size();
        names.push_back(name);
        films.emplace_back();
        nameToId.insert(name, id + 1);
        return id;
    }
    int lookup(const std::string& name) {      // get, or -1 if unknown
        int e = nameToId.search(name);
        return e == 0 ? -1 : e - 1;
    }

    void reconstruct(Result& r, const std::vector<int>& prevActor,
                     const std::vector<Movie*>& prevMovie, int s, int t) {
        std::vector<Step> rev;
        int cur = t;
        while (cur != s) {                     // walk backward from target
            rev.push_back({ prevMovie[cur], names[cur] });
            cur = prevActor[cur];
        }
        r.steps.assign(rev.rbegin(), rev.rend());   // reverse into forward order
        r.found = true;
    }
};

#endif