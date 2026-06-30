#ifndef CSVLOADER_H
#define CSVLOADER_H

#include <string>
#include <vector>
#include "Movie.h"

class CsvLoader {
public:
    static std::vector<Movie> load(const std::string& path);
};

#endif