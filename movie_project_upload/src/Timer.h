#ifndef TIMER_H
#define TIMER_H

#include <chrono>

class Timer {
public:
    void start() { t0 = std::chrono::steady_clock::now(); }

    // Elapsed time since start(), in nanoseconds.
    long long ns() const {
        auto t1 = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
    }

    double ms() const { return ns() / 1e6; }   // convenience for build times

private:
    std::chrono::steady_clock::time_point t0;
};

#endif