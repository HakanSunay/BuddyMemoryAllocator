//
// Created by Hakan Halil on 1.08.20.
//

#ifndef BUDDY_ALLOCATOR_BUDDYALLOCATOR_H
#define BUDDY_ALLOCATOR_BUDDYALLOCATOR_H

#include <iostream>
#include <cmath>
#include "Node.h"

// TODO: Add error handling
// TODO: See how can we handle memory leaks
// Interesting reads below:
// https://stackoverflow.com/questions/9074229/detecting-memory-leaks-in-c-programs
// @Semerdzhiev: "Leak data must also be kept in the Allocator's memory"
// For every allocation, I will have to keep __FILE__ & __LINE__, which will result in very big overhead
// TODO: Make Allocator super efficient:
// 1. Only bitshifts for arithmetic operations
// 2. Loop iter vars
// 3. Eliminate function call overheads (copying context & new stack frame)
// 4. Leaf functions
// 5. Inline functions
// Interesting reads below:
// https://www.codeproject.com/Articles/6154/Writing-Efficient-C-and-C-Code-Optimization
class Allocator {
    Node** freeLists;

    uint8_t* SplitTable;
    uint8_t* FreeTable;
    size_t TableSize;

    uint8_t *base_ptr;

    size_t max_memory_log;
    size_t max_memory_size;
    size_t actual_size;
    size_t actualVirtualSizeDiff;
    size_t actualVirtualSizeDiffRoundedToMinAlloc;

    size_t min_block_log = 4;
    size_t min_block_size = 1 << min_block_log;

    size_t free_list_count;
    size_t free_list_level_limit;

    size_t overheadSize;
    size_t overhead_blocks_count;
    size_t lastInnerStructureBlockIndex;

    size_t unusedSpace;
    size_t unusedBlocksCount;

    void initInnerStructures();

    inline size_t getBlockIndexFromAddr(uint8_t *ptr, size_t level);
    inline size_t getParentIndex(size_t index);
    inline uint8_t * getPtrFromBlockIndex(size_t index, size_t level);

    inline bool isRoot(size_t index);
    inline bool isLeftBuddy(size_t index);
    inline bool isRightBuddy(size_t index);
    inline size_t findBuddyIndex(size_t index);
    inline Node *FindRightBuddyOf(void *pVoid, int i);

    inline size_t calculateEnclosingPowerOf2Size(size_t size);
    inline size_t findBestFitIndex(size_t requested_memory);

    inline void markParentAsSplit(size_t index);
    inline void unmarkParentAsSplit(size_t index);
    inline bool isSplitBlockByIndex(size_t index);

    inline bool isFreeBlockBuddies(size_t index);
    inline void flipFreeTableIndexForBlockBuddies(size_t blockIndex);

    inline bool isSplitByAddrAndLevel(void *adr, size_t level);
    inline size_t findLevelOfAllocatedBlock(void* addr);

    inline size_t previousPowerOfTwo(size_t num);

    void printTree(uint8_t * arr, std::ostream& os, const char *mark);
    void exposeInnerStructures(std::ostream& os);
    void exposeFreeMemory(std::ostream& os);

    size_t round_up(size_t num, size_t factor);

    inline bool isInnerStructure(size_t index);

    inline bool isNotAllocated(size_t index, size_t i, void *pVoid);
public:
    Allocator(void* addr, size_t size);

    void* Allocate(size_t size);

    void Free(void* ptr);

    void Debug(std::ostream& os);

    static void CheckForLeaks();
};

#endif //BUDDY_ALLOCATOR_BUDDYALLOCATOR_H