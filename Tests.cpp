//
// Created by Hakan Halil on 2.08.20.
//

#include "Tests.h"
#include "BuddyAllocator.h"
#include <cstdlib>
#include <vector>

void StressTestWithAllocateAndFreeAgainstSystem() {
    std::cout << "Testing Allocation and Deallocation with 1MB of memory with static 32 byte allocations" << std::endl;
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
    a.Debug(std::cout);

    size_t countOfCalls = 3;
    int *nums[countOfCalls];
    for (int i = 0; i < countOfCalls; ++i) {
        nums[i] = (int*)a.Allocate(sizeof(int));
        *nums[i] = i;
    }

    std::cout << std::endl;
    a.Debug(std::cout);

    for (int i = 0; i < countOfCalls; ++i) {
        if (*nums[i] != i) {
            printf("OPA!!!");
        }
    }

    a.Free(nums[2]);
    a.Free(nums[1]);
    a.Free(nums[0]);

    std::cout << std::endl;
    a.Debug(std::cout);
}


void MixedTest() {
    size_t size = 25000;
    void * adr = malloc(size);
    Allocator a = Allocator(adr, size);
    a.Debug(std::cout);

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


    a.Debug(std::cout);
    free(adr);
}

void TestWithPowerOfTwo() {
    size_t size = 256;
    void * adr = malloc(size);
    Allocator a = Allocator(adr, size);

    a.Debug(std::cout);

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

void ProfileAllocator() {
    void *adr = malloc(1048576);

    Allocator a  = Allocator(adr, 1048576);

    int *nums[25000];

    auto start1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 25000; ++i) {
        nums[i] = (int*)a.Allocate(32);
        *nums[i] = i;
    }
    auto end1 = std::chrono::high_resolution_clock::now();


    auto dur1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1);


    std::cout << "Time taken by buddy to allocate: " << dur1.count() << " microseconds" << std::endl;

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
}

void TestHugeAllocations() {
    std::cout << "Testing Allocation and Deallocation with 1 GB of memory with ranging allocation sizes" << std::endl;
    // 1 GB - 1 073 741 824
    size_t oneGB = 1073741824;
    void *adr = malloc(oneGB);

    Allocator a  = Allocator(adr, oneGB);

    size_t countOfAllocs = 250000;

    // blocks of sizes allocBlockSize, allocBlockSize * 2, ..., allocBlockSize * blockDeviation
    // will be allocated sequentially
    size_t allocBlockSize = 32;
    size_t blockSizeDeviation = 4;


    // Allocation benchmark for buddy
    int *nums[countOfAllocs];
    auto buddyAllocStart = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < countOfAllocs; ++i) {
        nums[i] = (int*)a.Allocate(allocBlockSize * ((i % blockSizeDeviation) + 1));
        *nums[i] = i;
    }
    auto buddyAllocEnd = std::chrono::high_resolution_clock::now();
    auto buddyAllocDur = std::chrono::duration_cast<std::chrono::microseconds>(buddyAllocEnd - buddyAllocStart);
    std::cout << "Time taken by buddy to allocate: " << buddyAllocDur.count() << " microseconds" << std::endl;

    // Allocation benchmark for system
    int *m_nums[countOfAllocs];
    auto systemAllocStart = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < countOfAllocs; ++i) {
        m_nums[i] = (int*)malloc(allocBlockSize * ((i % blockSizeDeviation) + 1));
        *m_nums[i] = i;
    }
    auto systemAllocEnd = std::chrono::high_resolution_clock::now();
    auto systemAllocDur = std::chrono::duration_cast<std::chrono::microseconds>(systemAllocEnd - systemAllocStart);
    std::cout << "Time taken by system to allocate: " << systemAllocDur.count() << " microseconds" << std::endl;

    // Correctness check for buddy
    for (int i = 0; i < countOfAllocs; ++i) {
        if (*nums[i] != i) {
            printf("Expected %d, but got %d", i, *nums[i]);
        }
    }

    // Free benchmark for buddy
    auto buddyFreeStart = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < countOfAllocs; ++i) {
        a.Free(nums[i]);
    }
    auto buddyFreeEnd = std::chrono::high_resolution_clock::now();
    auto buddyFreeDur = std::chrono::duration_cast<std::chrono::microseconds>(buddyFreeEnd - buddyFreeStart);
    std::cout << "Time taken by buddy to free: " << buddyFreeDur.count() << " microseconds" << std::endl;

    //a.Debug(std::cout);


    // Free benchmark for system
    auto systemFreeStart = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < countOfAllocs; ++i) {
        free(m_nums[i]);
    }
    auto systemFreeEnd = std::chrono::high_resolution_clock::now();
    auto systemFreeDur = std::chrono::duration_cast<std::chrono::microseconds>(systemFreeEnd - systemFreeStart);
    std::cout << "Time taken by system to free: " << systemFreeDur.count() << " microseconds" << std::endl;

    free(adr);
    std::cout << "Ending test huge allocations" << std::endl;
}

void SimpleTest() {
    void *adr = malloc(1024);
    Allocator a = Allocator(adr, 1024);
    a.Debug(std::cout);

    // TODO: maybe testing like this is not really wise, because m[30] assumes continuous memory
    int *addresses[30];
    for (int i = 0; i < 30; ++i) {
        addresses[i] = (int*)a.Allocate(sizeof(int));
        *addresses[i] = i;
    }

    for (int i = 0; i < 30; ++i) {
        a.Free(addresses[i]);
    }

    a.Debug(std::cout);
    free(adr);
}

void TestSuperSmallAllocator() {
    void *adr = malloc(16);
    bool exceptionCaught = false;
    try {
        Allocator a = Allocator(adr, 16);
    } catch (char const* exception) {
        std::cout << "Expected exception was caught: " << exception << std::endl;
        exceptionCaught = true;
    }

    std::cout << "Testing invalid minimal size init " << (exceptionCaught ? "succeeded" : "failed") << std::endl;
}

void TestFreeInvalidAddress() {
    void *adr = malloc(32);
    Allocator a = Allocator(adr, 32);
    bool exceptionCaught = false;

    int randomNum = 5;
    int* randomDanglingPointer = &randomNum;

    try {
        a.Free(randomDanglingPointer);
    } catch (const char* exception) {
        std::cout << "Expected exception was caught: " << exception << std::endl;
        exceptionCaught = true;
    }

    std::cout << "Testing free with invalid address " << (exceptionCaught ? "succeeded" : "failed") << std::endl;
}

void TestAllocateWithSizeMoreThanManaged() {
    void *adr = malloc(32);
    Allocator a = Allocator(adr, 32);
    void* res = a.Allocate(120);
    std::cout << "Testing allocate with unexpected size " << (res == nullptr ? "succeeded" : "failed") << std::endl;
}
