#include <iostream>
#include <chrono>
#include "BuddyAllocator.h"

int main() {
    size_t size = 200;
    void * adr = malloc(size);
    Allocator a = Allocator(adr, size);

    std::cout << std::endl;
    a.debug(std::cout);

    size_t countOfCalls = 3;
    int *nums[countOfCalls];
    for (int i = 0; i < countOfCalls; ++i) {
        nums[i] = (int*)a.Allocate(sizeof(int));
        *nums[i] = i;
    }

    std::cout << std::endl;
    a.debug(std::cout);

    for (int i = 0; i < countOfCalls; ++i) {
        if (*nums[i] != i) {
            printf("OPA!!!");
        }
    }

    a.Free(nums[2]);
    a.Free(nums[1]);
    a.Free(nums[0]);

    std::cout << std::endl;
    a.debug(std::cout);

//    void *adr = malloc(1048576);
//
//    Allocator a  = Allocator(adr, 1048576);
//
//    // TODO: Stress testing with bigger allocator size and different allocation sizes
//    // Possible regression might happen in FreeTable values, but debugging will be hard
//
//    int *nums[25000];
//
//    auto start1 = std::chrono::high_resolution_clock::now();
//    for (int i = 0; i < 25000; ++i) {
//        nums[i] = (int*)a.Allocate(32);
//        *nums[i] = i;
//    }
//    auto end1 = std::chrono::high_resolution_clock::now();
//
//    int *m_nums[25000];
//    auto start2 = std::chrono::high_resolution_clock::now();
//    for (int i = 0; i < 25000; ++i) {
//        m_nums[i] = (int*)malloc(32);
//        *m_nums[i] = i;
//    }
//    auto end2 = std::chrono::high_resolution_clock::now();
//
//    auto dur1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1);
//    auto dur2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2);
//
//
//    std::cout << "Time taken by buddy: " << dur1.count() << " microseconds" << std::endl;
//    std::cout << "Time taken by malloc: " << dur2.count() << " microseconds" << std::endl;
//
//    // Correctness check
//    for (int i = 0; i < 25000; ++i) {
//        if (*nums[i] != i) {
//            printf("OPA!!!");
//        }
//    }

//
//    for (int i = 0; i < 1000; ++i) {
//        a.Free(nums[999-i]);
//    }
//
//    int* medNum = (int*)a.Allocate(32);
//    *medNum = 3;
//    int* bigNum = (int*)a.Allocate(64);
//    *bigNum = 4;
//
//    a.Free(bigNum);
//    a.Free(medNum);
//
//
////    a.Free(nums[2]);
////    a.Free(nums[1]);
//
//    std::cout << "Hello, World!" << std::endl;
//    free(adr);
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