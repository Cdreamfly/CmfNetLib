#include <iostream>
#include "base/Log.hpp"
#include <chrono>

class Timer {
public:
    std::chrono::time_point<std::chrono::steady_clock> start, end;
    std::chrono::duration<float> duration{};

    Timer() {
        start = std::chrono::steady_clock::now();
    }

    ~Timer() {
        end = std::chrono::steady_clock::now();
        duration = end - start;
        float ms = duration.count() * 1000.0f;
        std::cout << "Timer took " << ms << " ms" << std::endl;
    }
};


int main() {
    func();
    return 0;
}