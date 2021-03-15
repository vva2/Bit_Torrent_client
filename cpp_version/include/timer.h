#pragma once
#include <chrono>
#include <iostream>

class Timer {
    public:
        std::chrono::high_resolution_clock::time_point start, end;
        Timer() {
            start = std::chrono::high_resolution_clock::now();
        }
        ~Timer() {
            std::chrono::high_resolution_clock::duration taken = std::chrono::high_resolution_clock::now()-start;
            std::cout << taken.count()  << "\n";
        }
};