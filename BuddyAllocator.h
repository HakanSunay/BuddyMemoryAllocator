//
// Created by Hakan Halil on 1.08.20.
//

#include <iostream>
#include <cmath>
#include <chrono>
#include "Node.h"

#ifndef UNTITLED_NEWEST_BUDDYALLOCATOR_H
#define UNTITLED_NEWEST_BUDDYALLOCATOR_H

// TODO: Modularize code
// TODO: Add debug option which will print current state of the allocator:
// Allocated items - size and address
// Free spaces - size and address
// TODO: See how can we handle memory leaks
// Interesting reads below:
// https://stackoverflow.com/questions/9074229/detecting-memory-leaks-in-c-programs
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

    // TODO: Is this really necessary?
    uint8_t *offset_ptr;

    size_t max_memory_log;
    size_t max_memory_size;

    size_t min_block_log = 4;
    size_t min_block_size = 1 << min_block_log;

    size_t free_list_count;
    size_t free_list_level_limit;

    size_t overheadSize;
    size_t overhead_blocks_count;

public:
    size_t getBlockIndexFromAddr(uint8_t *ptr, size_t limit) {
        return ((ptr - base_ptr) >> (max_memory_log - limit)) + (1 << limit) - 1;
    }

    size_t getParentIndex(size_t index) {
        return (index - 1) / 2;
    }

    uint8_t * getPtrFromBlockIndex(size_t index, size_t level) {
        return base_ptr + ((index - (1 << level) + 1) << (this->max_memory_log - level));
    }

    bool isRoot(size_t index) {
        return index == 0;
    }

    bool isLeftBuddy(size_t index) {
        return index % 2 == 1;
    }

    bool isRightBuddy(size_t index) {
        return index % 2 == 0;
    }

    void initInnerStructures() {
        // last level first index => 2^max_level - 1
        size_t firstUnusedBlockIndex = (1 << free_list_level_limit) - 1;
        size_t tempUnusedBlockIndex = firstUnusedBlockIndex;
        // Must add extra unused blocks first
        for (int i = 0; i < this->unusedBlocksCount; ++i) {
            markParentAsSplit(tempUnusedBlockIndex);
            flipFreeTableIndexForBlockBuddies(tempUnusedBlockIndex);
            tempUnusedBlockIndex++;
        }


        size_t lastInnerStructureBlockIndex = getBlockIndexFromAddr(base_ptr + (min_block_size * (overhead_blocks_count - 1)), free_list_level_limit);
        // take unused blocks into consideration
        lastInnerStructureBlockIndex += unusedBlocksCount;
        size_t currentBlockIndex = lastInnerStructureBlockIndex;
        size_t currentLevel = free_list_level_limit;


        size_t tempIndex = currentBlockIndex;
        for (int i = 0; i < this->overhead_blocks_count; ++i) {
            markParentAsSplit(tempIndex);
            flipFreeTableIndexForBlockBuddies(tempIndex);
            tempIndex--;
        }

        // while not at root index
        while (!isRoot(currentBlockIndex)) {
            if (isRightBuddy(currentBlockIndex)) {
                // nothing to do in this case
            } else if (isLeftBuddy(currentBlockIndex)) {
                void *ptr = getPtrFromBlockIndex(currentBlockIndex, currentLevel);
                // TODO:
                // This function is not really safe, I can replace with index & level -> ptr
                Node *rightBuddy = FindRightBuddyOf(ptr, max_memory_log - currentLevel);
                rightBuddy->next = nullptr;

                PushNewNode(&this->freeLists[currentLevel], rightBuddy);
                flipFreeTableIndexForBlockBuddies(currentBlockIndex);
            }
            markParentAsSplit(currentBlockIndex);
            currentBlockIndex = getParentIndex(currentBlockIndex);
            currentLevel--;
        }
    }

    Allocator(void* addr, size_t size) {
        base_ptr = (uint8_t *)(addr);
        offset_ptr = base_ptr;

        // max_memory_size is the actual size
        max_memory_size = size;

        // max_memory_log is the ceiled log, example:
        // 500 size -> log2(512)
        max_memory_log = ceil(log2(size));
        unusedSpace = (1 << max_memory_log) - size;

        // 4 for current test
        unusedBlocksCount = ceil(unusedSpace / float(min_block_size));

        free_list_count = max_memory_log - min_block_log + 1;
        free_list_level_limit = free_list_count - 1;

        // init free lists
        freeLists = (Node**) base_ptr;
        for (int i = 0; i < free_list_count; ++i) {
            freeLists[i] = ((Node*) base_ptr) + i;
            freeLists[i] = nullptr;
        }

        // HERE: addr + 3 -> NEXT: addr + 4 ( 4 x 8 )
        // TODO: I can probably get the last index of freeList and incr it by sizeof(Node*) [8 bytes]
        this->SplitTable = (uint8_t *) (base_ptr) + free_list_count * sizeof(Node *);
        this->TableSize = ((unsigned) 1 << (free_list_count - 1)) / 8;
        for (int j = 0; j < TableSize; ++j) {
            this->SplitTable[j] = 0;
        }

        // TODO: I can probably get the last index of SplitTable and incr it by sizeof(uint8_t) [1 byte]
        this->FreeTable = (u_int8_t *) (base_ptr) + (free_list_count * sizeof(Node *)) + (TableSize * sizeof(uint8_t));
        for (int k = 0; k < TableSize; ++k) {
            this->FreeTable[k] = 0;
        }

        this->overheadSize = (free_list_count * sizeof(Node *)) + (this->TableSize * sizeof (uint8_t)) + (this->TableSize * sizeof (uint8_t));
        this->overhead_blocks_count = ceil(overheadSize / float(min_block_size));

        // updates all necessary inner structures with current state
        initInnerStructures();
    }

    Node *FindRightBuddyOf(void *pVoid, int i) {
        // we have to shift 2^i bytes to the right to find the buddy
        return (Node*)(((uint8_t *)pVoid) + (1 << i));
    }

    size_t calculateEnclosingPowerOf2Size(size_t size) {
        return (unsigned) 1 << unsigned (floor(log2(size)));
    }

    size_t findBestFitIndex(size_t requested_memory) {
        size_t free_list_index = free_list_count - 1;
        size_t size = this->min_block_size;

        while (size < requested_memory) {
            free_list_index--;
            size *= 2;
        }

        return free_list_index;
    }

    void markParentAsSplit(size_t index) {
        index = (index - 1) / 2;
        SplitTable[index / 8] |= (unsigned)1 << (index % 8);
    }

    // 000 111 000
    // &
    // 111 001 111
    // ....
    void unmarkParentAsSplit(size_t index) {
        index = (index - 1) / 2;
        SplitTable[index / 8] &= ~((unsigned)1 << (index % 8));
    }

    bool isSplitBlockByIndex(size_t index) {
        return (SplitTable[index / 8] >> (index % 8)) & 1;
    }

    bool isFreeBlockBuddies(size_t index) {
        index = (index - 1) / 2;
        return (FreeTable[index / 8] >> (index % 8)) & 1;
    }

    void flipFreeTableIndexForBlockBuddies(size_t blockIndex) {
        size_t index = (blockIndex - 1) / 2;
        FreeTable[index / 8] ^= (unsigned)1 << (index % 8);
    }

    bool isSplitByAddrAndLevel(void *adr, size_t level) {
        size_t blockIndex = getBlockIndexFromAddr((uint8_t *)(adr), level);
        return isSplitBlockByIndex(blockIndex);
    }

    size_t findLevelOfAllocatedBlock(void* addr) {
        size_t currentLevel = free_list_level_limit;
        while (currentLevel > 0) {
            if (isSplitByAddrAndLevel(addr, currentLevel - 1)) {
                return currentLevel;
            }
            currentLevel--;
        }
        return 0;
    }

    void* Allocate(size_t size) {
        int i = findBestFitIndex(size);

        // this is currently hit only if we received more than max, but we need to take care of the overhead sizes as well
        if (this->max_memory_log - i >= this->max_memory_log) {
            return nullptr;
        } else if (this->freeLists[i] != nullptr) {
            // we have a block with this size in the free list
            void * res = Pop(&this->freeLists[i]);
            size_t blockIndexOfRes = getBlockIndexFromAddr((uint8_t*)res, i);

            // TODO: NOT SURE IF THESE SHOULD BE HERE
            markParentAsSplit(blockIndexOfRes);
            flipFreeTableIndexForBlockBuddies(blockIndexOfRes);

            return res;
        } else {
            // we need to split a bigger block
            void * block = Allocate(1 << (max_memory_log - i + 1));
            if (block != nullptr) {
                size_t blockIndex = getBlockIndexFromAddr((uint8_t*)block, i);
                // with the allocate on top we are getting the bigger chunk
                // split and put the extra (right child) to the free list, which is in the current level

                size_t buddyIndex = findBuddyIndex(blockIndex);
                Node * buddy = (Node *)getPtrFromBlockIndex(buddyIndex, i);
                buddy->next = nullptr;

                PushNewNode(&this->freeLists[i], (Node*)buddy);

                // TODO: NOT SURE IF THESE SHOULD BE HERE
                markParentAsSplit(blockIndex);
                flipFreeTableIndexForBlockBuddies(blockIndex);
            }
            return block;
        }
    }

    size_t findBuddyIndex(size_t index) {
        // root node
        if (index == 0) {
            return 0;
        }

        return ((index - 1) ^ 1) + 1;
    }

    void Free(void* ptr) {
        size_t allocationLevel = findLevelOfAllocatedBlock(ptr);
        size_t allocationSize = 1 << (max_memory_log - allocationLevel);
        size_t blockIndex = getBlockIndexFromAddr((uint8_t*)ptr, allocationLevel);

        size_t currentLevel = allocationLevel;
        size_t currentIndex = blockIndex;

        // traversing upwards
        while (!isRoot(currentIndex)) {
            bool isFreeBuddy = isFreeBlockBuddies(currentIndex);
            if (!isFreeBuddy) {
                // stopping here and will add ourselves to the free lists of our level
                // Current block will become free therefore we flip
                flipFreeTableIndexForBlockBuddies(currentIndex);
                break;
            }

            // Current block will become free therefore we flip
            flipFreeTableIndexForBlockBuddies(currentIndex);

            // we will certainly go 1 level up, so parent will no longer be split
            unmarkParentAsSplit(currentIndex);

            // our buddy is free, therefore we need to remove it from its free list
            // adding to the actual new free list will be done out of the loop
            size_t buddyIndex = findBuddyIndex(currentIndex);
            Node *buddyNode = (Node*)getPtrFromBlockIndex(buddyIndex, currentLevel);
            RemoveNode(&this->freeLists[currentLevel], buddyNode);
            currentIndex = (currentIndex - 1) / 2;
            currentLevel--;
        }

        Node* newNode = (Node*)getPtrFromBlockIndex(currentIndex, currentLevel);
        newNode->next = nullptr;
        PushNewNode(&this->freeLists[currentLevel], newNode);
    }

    size_t unusedSpace;
    size_t unusedBlocksCount;
};

#endif //UNTITLED_NEWEST_BUDDYALLOCATOR_H