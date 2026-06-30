#ifndef RESTRICTIONS_H
#define RESTRICTIONS_H

#include <cstddef>

// A single place to turn restrictions on/off, so the SAME code path runs
// both with and without them. Flip a flag, rerun, compare the numbers.
struct Restrictions {
    // R2 (memory): cap how many elements a structure will accept. 0 = no cap.
    size_t maxElements = 0;

    // R12 (latency): microseconds of artificial delay per operation. 0 = none.
    unsigned latencyMicros = 0;

    // R16 (data): fraction of records to corrupt/drop, 0.0..1.0. 0 = clean.
    double corruptFraction = 0.0;

    // R24 (algorithm): if true, ranking uses selection sort instead of std::sort.
    bool useSlowSort = false;

    // R7 (processing): max nanoseconds allowed per operation; operations slower
    // than this count as budget violations. 0 = no budget enforced.
    long long opBudgetNs = 0;

    // R6 (processing) is enforced outside the code (core pinning) and only
    // recorded here as documentation of the active condition.
    bool singleCore = false;
};

#endif