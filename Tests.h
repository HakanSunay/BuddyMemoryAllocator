//
// Created by Hakan Halil on 1.08.20.
//

#ifndef UNTITLED_NEWEST_TESTS_H
#define UNTITLED_NEWEST_TESTS_H


#include <cstdlib>
#include "BuddyAllocator.h"

void StressTestWithAllocateAndFreeAgainstSystem() {
    void *adr = malloc(1048576);

    Allocator a  = Allocator(adr, 1048576);

    // TODO: Stress testing with bigger allocator size and different allocation sizes
    // Possible regression might happen in FreeTable values, but debugging will be hard

    int *nums[25000];

    auto start1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 25000; ++i) {
        nums[i] = (int*)a.Allocate(32);
        *nums[i] = i;
    }
    auto end1 = std::chrono::high_resolution_clock::now();

    int *m_nums[25000];
    auto start2 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 25000; ++i) {
        m_nums[i] = (int*)malloc(32);
        *m_nums[i] = i;
    }
    auto end2 = std::chrono::high_resolution_clock::now();

    auto dur1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1);
    auto dur2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2);


    std::cout << "Time taken by buddy to allocate: " << dur1.count() << " microseconds" << std::endl;
    std::cout << "Time taken by system to allocate: " << dur2.count() << " microseconds" << std::endl;

    // Correctness check
    for (int i = 0; i < 25000; ++i) {
        if (*nums[i] != i) {
            printf("Expected %d, but got %d", i, *nums[i]);
        }
    }


    // Free check
    auto freeStart1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 25000; ++i) {
        a.Free(nums[i]);
    }
    auto freeEnd1 = std::chrono::high_resolution_clock::now();
    auto freeDur1 = std::chrono::duration_cast<std::chrono::microseconds>(freeEnd1 - freeStart1);
    std::cout << "Time taken by buddy to free: " << freeDur1.count() << " microseconds" << std::endl;


    // Free check
    auto freeStart2 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 25000; ++i) {
        free(m_nums[i]);
    }
    auto freeEnd2 = std::chrono::high_resolution_clock::now();
    auto freeDur2 = std::chrono::duration_cast<std::chrono::microseconds>(freeEnd2 - freeStart2);
    std::cout << "Time taken by system to free: " << freeDur2.count() << " microseconds" << std::endl;
}

void DebugTest() {
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
}

void MixedTest() {
    size_t size = 25000;
    void * adr = malloc(size);
    Allocator a = Allocator(adr, size);

    size_t countOfCalls = 1000;
    int *nums[countOfCalls];
    for (int i = 0; i < countOfCalls; ++i) {
        nums[i] = (int*)a.Allocate(sizeof(int));
        *nums[i] = i;
    }

    for (int i = 0; i < countOfCalls; ++i) {
        if (*nums[i] != i) {
            printf("OPA!!!");
        }
    }


    for (int i = 0; i < 1000; ++i) {
        a.Free(nums[999-i]);
    }

    int* medNum = (int*)a.Allocate(32);
    *medNum = 3;
    int* bigNum = (int*)a.Allocate(64);
    *bigNum = 4;

    a.Free(bigNum);
    a.Free(medNum);


    a.Free(nums[2]);
    a.Free(nums[1]);
    free(adr);
}

void TestWithPowerOfTwo() {
    size_t size = 256;
    void * adr = malloc(size);
    Allocator a = Allocator(adr, size);

    size_t countOfCalls = 3;
    int *nums[countOfCalls];
    for (int i = 0; i < countOfCalls; ++i) {
        nums[i] = (int*)a.Allocate(sizeof(int));
        *nums[i] = i;
    }

    for (int i = 0; i < countOfCalls; ++i) {
        if (*nums[i] != i) {
            printf("OPA!!!");
        }
    }

    for (int i = 0; i < countOfCalls; ++i) {
        a.Free(nums[countOfCalls-i]);
    }

    int* medNum = (int*)a.Allocate(32);
    *medNum = 3;
    int* bigNum = (int*)a.Allocate(64);
    *bigNum = 4;

    a.Free(bigNum);
    a.Free(medNum);
    a.Free(nums[2]);
    a.Free(nums[1]);

    free(adr);
}

#endif //UNTITLED_NEWEST_TESTS_H
