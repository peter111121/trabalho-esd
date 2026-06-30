cmake_minimum_required(VERSION 3.20)
project(movie_ds_project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(movie_ds_project
        src/main.cpp
        src/CsvLoader.cpp
        src/CsvLoader.h
        src/Movie.h
        src/HashTable.h
        src/LinkedList.h
        src/ActorIndex.h
        src/AVLTree.h
        src/YearIndex.h
        src/Analytics.h
        src/MovieTrie.h
        src/MovieTrie.cpp
        src/PatriciaTrie.h
        src/PatriciaTrie.cpp
        src/UnionFind.h
        src/ActorGraph.h
        src/CollaborationPath.h
        src/Timer.h
        src/Benchmark.h
        src/BenchmarkB.h
        src/BenchmarkC.h
        src/RelationshipQueries.h
        src/Menu.h
        src/Restrictions.h
        src/Corruptor.h
        src/SlowSort.h
        src/BenchmarkRestrictions.h
)

target_link_options(movie_ds_project PRIVATE -static -static-libgcc -static-libstdc++)