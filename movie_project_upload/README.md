# Movie Data System — Data Structures Project 2026

A C++ system that ingests the Rotten Tomatoes movie dataset and indexes it with
several data structures, then compares their performance under controlled
benchmarks and restriction scenarios. Built for the Estrutura de Dados 2026
project.

## What it does

The system loads ~17,000 movies and supports, through an interactive terminal
menu:

- Find a movie by exact title (hash table)
- Search titles by prefix / autocomplete (trie)
- List an actor's filmography (hash table of linked lists)
- Rating statistics (mean, standard deviation) for a year range (AVL tree)
- Top-N movies by rating in a year range (AVL tree + sort)
- Actor relationships answered three ways: direct collaboration, network
  connectivity (Union-Find), and shortest collaboration path (BFS)

## Data structures implemented

Classical: hash table (separate chaining), AVL tree, singly linked list.
Outside the syllabus: trie and Union-Find (with path compression and union by
rank). Optimization: a Patricia/radix trie compared against the plain trie.

## Dataset

This project uses the "Rotten Tomatoes movies and critic reviews dataset"
(author: stefanoleone992) from Kaggle. The dataset file is NOT included in this
repository because of its size.

To run the project:

1. Download the dataset from Kaggle.
2. Place `rotten_tomatoes_movies.csv` in the `data/` folder.

The loader reads columns by name from the CSV header, so the standard column
layout of that dataset is expected.

## Build and run

Requirements: a C++17 compiler and CMake 3.20+. Developed in CLion.

```
cmake -S . -B build
cmake --build build
```

Two notes when running:

- Set the working directory to the project root (the folder containing
  `data/`), so the relative path `data/rotten_tomatoes_movies.csv` resolves.
  In CLion: Run > Edit Configurations > Working directory.
- Build and run in Release mode. Benchmark timings are only meaningful with
  compiler optimizations enabled, and Release avoids some Windows
  application-control blocking of fresh debug binaries.

## Project layout

- `src/` — all source and header files
- `data/` — place the dataset CSV here (not committed)
- `CMakeLists.txt` — build configuration

## Use of generative AI

Generative AI was used for support during coding, in accordance with the
project rules. The report was written without generative AI. The full
conversation(s) that produced code, including all prompts, are linked below:

- https://claude.ai/share/d585ea3c-d62c-4185-a420-d0fba841c309
- https://claude.ai/share/a80ca4c6-6ea2-4c87-9bff-a5f22df394c1
- https://claude.ai/share/8d43d482-878b-4974-ace1-5c018c2b6efe

