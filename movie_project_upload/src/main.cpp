#include <iostream>
#include "CsvLoader.h"
#include "Menu.h"

int main() {
    std::vector<Movie> movies = CsvLoader::load("data/rotten_tomatoes_movies.csv");
    if (movies.empty()) { std::cerr << "No data loaded.\n"; return 1; }

    Menu menu(movies);
    menu.run();
    return 0;
}
