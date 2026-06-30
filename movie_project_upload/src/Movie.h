#ifndef MOVIE_H
#define MOVIE_H

#include <string>
#include <vector>

struct Movie {
    std::string id;                     // unique link/key from the dataset
    std::string title;
    int year = -1;                      // -1 means "unknown"
    std::vector<std::string> genres;    // e.g. {"Action", "Comedy"}
    std::string director;
    std::vector<std::string> actors;    // e.g. {"Tom Hanks", "Tim Allen"}
    double criticRating = -1.0;         // tomatometer
    double audienceRating = -1.0;
    int criticCount = -1;               // how many critics reviewed it
    int audienceCount = -1;             // how many audience ratings
    int runtime = -1;
};

#endif