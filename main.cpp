#include <iostream>
#include <chrono>
#include "BuddyAllocator.h"
#include "Tests.h"

int main() {
    std::cout << "Hakan's Buddy Memory Allocator main started" << std::endl;
    TestWithPowerOfTwo();
    return 0;
}

// Sample results:
// Only once was my buddy quicker
// Time taken by buddy: 2075 microseconds
// Time taken by malloc: 2128 microseconds
// Other cases
// Time taken by buddy: 2091 microseconds
// Time taken by malloc: 1951 microseconds
// ---------------------------------------
// Time taken by buddy: 2708 microseconds
// Time taken by malloc: 2165 microseconds