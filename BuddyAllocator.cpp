//
// Created by Hakan Halil on 1.08.20.
//

#include "BuddyAllocator.h"
#include <iostream>
#include <cmath>
#include "Node.h"

const char* BUDDY_INIT_EXCEPTION_MSG = "Allocator cannot be initialized with size less than twice of minimum block size";
const char* BUDDY_INIT_WITH_NULLPTR_EXCEPTION_MSG = "Allocator cannot be initialized using nullptr as the memory block to be managed";
const char* BUDDY_FREE_EXCEPTION_MSG = "Input address is not managed by the Allocator";

Allocator::Allocator(void *addr, size_t size) {
    if (addr == nullptr) {
        throw Exception(BUDDY_INIT_WITH_NULLPTR_EXCEPTION_MSG);
    }

    if (size < min_block_size * 2) {
        // Is throwing exception in constructor a good practice?
        throw Exception(BUDDY_INIT_EXCEPTION_MSG);
    }

    base_ptr = (uint8_t *)(addr);

    actual_size = size;

    // max_memory_log is the ceiled log, example:
    // 500 size -> log2(512)
    max_memory_log = floor(log2(size)); //ceil(log2(size));
    max_memory_size = (1 << max_memory_log);

    actualVirtualSizeDiff = 0; //max_memory_size - actual_size;
    actualVirtualSizeDiffRoundedToMinAlloc = 0; //round_up(actualVirtualSizeDiff, min_block_size);

    unusedSpace = max_memory_size - actual_size;

    // 4 for current test
    unusedBlocksCount = 0; // ceil(unusedSpace / float(min_block_size));

    free_list_count = max_memory_log - min_block_log + 1;
    free_list_level_limit = free_list_count - 1;

    // init free lists
    freeLists = (Node**) base_ptr;
    for (int i = 0; i < free_list_count; ++i) {
        freeLists[i] = ((Node*) base_ptr) + i;
        freeLists[i] = nullptr;
    }

    // TODO: I can probably get the last index of freeList and incr it by sizeof(Node*) [8 bytes]
    this->SplitTable = (uint8_t *) (base_ptr) + free_list_count * sizeof(Node *);
    this->TableSize = ((unsigned) 1 << (free_list_count - 1)) / 8;
    if (this->TableSize == 0 ) {this->TableSize = 1;}
    for (int j = 0; j < TableSize; ++j) {
        this->SplitTable[j] = 0;
    }

    // TODO: I can probably get the last index of SplitTable and incr it by sizeof(uint8_t) [1 byte]
    this->FreeTable = (uint8_t *) (base_ptr) + (free_list_count * sizeof(Node *)) + (TableSize * sizeof(uint8_t));
    for (int k = 0; k < TableSize; ++k) {
        this->FreeTable[k] = 0;
    }

    this->overheadSize = (free_list_count * sizeof(Node *)) + (this->TableSize * sizeof (uint8_t)) + (this->TableSize * sizeof (uint8_t));
    this->overhead_blocks_count = ceil(overheadSize / float(min_block_size));

    // updates all necessary inner structures with current state
    initInnerStructures();
}

// TODO: Unused block logic is broken
// TODO: Alignment
void Allocator::initInnerStructures() {
    // last level first index => 2^max_level - 1
    size_t firstUnusedBlockIndex = (1 << free_list_level_limit) - 1;
    size_t tempUnusedBlockIndex = firstUnusedBlockIndex;
    // Must add extra unused blocks first
    for (int i = 0; i < this->unusedBlocksCount; ++i) {
        markParentAsSplit(tempUnusedBlockIndex);
        tempUnusedBlockIndex++;
    }

    lastInnerStructureBlockIndex = getBlockIndexFromAddr(base_ptr + (min_block_size * (overhead_blocks_count - 1)), free_list_level_limit);
    // take unused blocks into consideration
    lastInnerStructureBlockIndex += unusedBlocksCount;
    size_t currentBlockIndex = lastInnerStructureBlockIndex;
    size_t currentLevel = free_list_level_limit;


    size_t tempIndex = currentBlockIndex;
    for (int i = 0; i < this->overhead_blocks_count; ++i) {
        markParentAsSplit(tempIndex);
        tempIndex--;
    }

    // while not at root index
    while (!isRoot(currentBlockIndex)) {
        if (isRightBuddy(currentBlockIndex)) {
            // nothing to do in this case
        } else if (isLeftBuddy(currentBlockIndex)) {
            void *ptr = getPtrFromBlockIndex(currentBlockIndex, currentLevel);
            // TODO: This function is not really safe, I can replace with index & level -> ptr
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

void *Allocator::Allocate(size_t size) {
    int i = findBestFitIndex(size);

    // practically speaking, we can never allocate as much as our whole managed memory space,
    // because we will always have inner structures at lowest level,
    // which in turn means we support at max (1 << max_memory_log - 1) sized allocations, therefore we use >=
    // in other words, if we get i = 0, we will fail - return nullptr
    if (this->max_memory_log - i >= this->max_memory_log) {
        return nullptr;
    } else if (this->freeLists[i] != nullptr) {
        // we have a block with this size in the free list
        void * res = Pop(&this->freeLists[i]);
        size_t blockIndexOfRes = getBlockIndexFromAddr((uint8_t*)res, i);

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

            markParentAsSplit(blockIndex);
            flipFreeTableIndexForBlockBuddies(blockIndex);
        }
        return block;
    }
}

void Allocator::Free(void *ptr) {
    // Very interesting read by Raymond Chen, which suggests that comparisons on pointers is not well defined:
    // https://devblogs.microsoft.com/oldnewthing/20170927-00/?p=97095
    // TODO: Define custom exception class
//    if (((uintptr_t) ptr < (uintptr_t)base_ptr) || ((uintptr_t) ptr > (uintptr_t)base_ptr + actual_size)) {
//        throw Exception(BUDDY_FREE_EXCEPTION_MSG);
//    }
    size_t allocationLevel = findLevelOfAllocatedBlock(ptr);
    // can be used for debugging
    // size_t allocationSize = 1 << (max_memory_log - allocationLevel);
    size_t blockIndex = getBlockIndexFromAddr((uint8_t*)ptr, allocationLevel);

    size_t currentLevel = allocationLevel;
    size_t currentIndex = blockIndex;

    #ifdef DEBUG
    if (isInnerStructure(blockIndex) || isNotAllocated(blockIndex, allocationLevel, ptr)) {
        return;
    }
    #endif

    // traversing upwards
    while (!isRoot(currentIndex)) {
        bool isFreeBuddy = isFreeBlockBuddies(currentIndex);
        if (!isFreeBuddy) {
            // stopping here and will add ourselves to the free lists of our level
            // current block will become free therefore we flip
            flipFreeTableIndexForBlockBuddies(currentIndex);
            break;
        }

        // we will certainly go 1 level up, so parent will no longer be split
        unmarkParentAsSplit(currentIndex);
        // current block will become free therefore we flip
        flipFreeTableIndexForBlockBuddies(currentIndex);

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

void Allocator::printTree(uint8_t *arr, std::ostream &os, const char *mark) {
    std::string serializedTree;
    for (int j = 0; j < this->TableSize; ++j) {
        uint8_t splitByte = arr[j];
        for (int i = 0; i < 8; ++i, splitByte >>= 1) {
            if (splitByte & 0x1) {
                serializedTree.append("1");
            } else {
                serializedTree.append("0");
            }
        }
    }

    // the string are read from left to right, unlike the bit map
    // first char is 0th bit ( 2 ^ 0)

    size_t ix = 0;
    for (int k = 0; k < this->free_list_level_limit; ++k) {
        size_t countOfElementsToBePrinted = 1 << k;
        os << (1 << (max_memory_log - k)) << ": \t\t";

        for (int i = 0; i < countOfElementsToBePrinted; ++i) {
            os << ((serializedTree[ix] == '1') ? mark : "_") << " ";
            ix++;
        }
        os << "\n";
    }

    os << "\n";
}

void Allocator::Debug(std::ostream &os) {
    os << "Debug information: \n\n";
    os << "Asked size was: " << this->max_memory_size - this->unusedSpace << std::endl;
    os << "Virtual size was: " << this->max_memory_size << std::endl;
    os << "Size after inner structures was: " << this->max_memory_size - this->unusedBlocksCount * this->min_block_size - (this->overhead_blocks_count * min_block_size) << std::endl << std::endl;

    exposeFreeMemory(os);
}

void Allocator::exposeInnerStructures(std::ostream &os) {
    os << "Inner structures details:" << "\n";
    uint8_t * addrIter = base_ptr;
    os << "Address space range: [" << (void*)addrIter << ";" << (void*)((uint8_t *)addrIter + this->actual_size) << "]" << std::endl;
    for (int j = 0; j < this->free_list_count; ++j) {
        os << "Free list " << j + 1 << " : " << (void*)addrIter << std::endl;
        addrIter += sizeof(Node*);
    }

    os << "Split table structure details:" << "\n";
    for (int j = 0; j < this->TableSize; ++j) {
        os << "Split Tables " << j + 1 << " : " << (void*)addrIter << std::endl;
        addrIter += sizeof(uint8_t*);
    }
    os << "Split table status: " << "\n";
    printTree(this->SplitTable, os, "S");

    os << "Free table structure details:" << "\n";
    for (int j = 0; j < this->TableSize; ++j) {
        os << "Free Tables " << j + 1 << " : " << (void*)addrIter << std::endl;
        addrIter += sizeof(uint8_t*);
    }
    os << "Free table status: " << "\n";
    printTree(this->FreeTable, os, "F");
}

void Allocator::exposeFreeMemory(std::ostream &os) {
    size_t totalFreeMemory = 0;
    for (int i = 0; i < this->free_list_count; ++i) {
        size_t freeBlockCountOnCurrentLevel = GetLength(&freeLists[i]);
        os << freeBlockCountOnCurrentLevel << " available block" << (freeBlockCountOnCurrentLevel != 1 ? "s" : "") << " with size " << (1 << (max_memory_log - i)) << "\n";
        totalFreeMemory += ((1 << (max_memory_log - i)) * freeBlockCountOnCurrentLevel);
    }

    // TODO: max_size --> to actual_size if any memory alloc is supported
    os << "Free memory size as of now: " << totalFreeMemory << "\n";
    os << "Total allocated memory size as of now: " << this->max_memory_size - totalFreeMemory << "\n";
    os << "Allocated for inner structures: " << this->overhead_blocks_count * min_block_size << "\n";
    os << "Allocated for users: " << (this->max_memory_size - totalFreeMemory) - (this->overhead_blocks_count * min_block_size) - (this->actualVirtualSizeDiffRoundedToMinAlloc - this->actualVirtualSizeDiff) << "\n\n";
}

void Allocator::CheckForLeaks() {
    std::cout<<"CHECKING AT EXIT" << std::endl;
}

size_t Allocator::getBlockIndexFromAddr(uint8_t *ptr, size_t level) {
    // find offset between base_ptr and input ptr
    // divide to find index offset from first block on that level
    // add to first index on that level
    return ((ptr - base_ptr) >> (max_memory_log - level)) + (1 << level) - 1;
}

size_t Allocator::getParentIndex(size_t index) {
    return (index - 1) / 2;
}

uint8_t *Allocator::getPtrFromBlockIndex(size_t index, size_t level) {
    // find index offset from first index on that level
    // multiply it by block size on that level
    // add resulting bytes offset to base_ptr
    return base_ptr + ((index - (1 << level) + 1) << (this->max_memory_log - level));
}

bool Allocator::isRoot(size_t index) {
    return index == 0;
}

bool Allocator::isLeftBuddy(size_t index) {
    return index % 2 == 1;
}

bool Allocator::isRightBuddy(size_t index) {
    return index % 2 == 0;
}

Node *Allocator::FindRightBuddyOf(void *pVoid, int i) {
    // we have to shift 2^i bytes to the right to find the buddy
    return (Node*)(((uint8_t *)pVoid) + (1 << i));
}

size_t Allocator::calculateEnclosingPowerOf2Size(size_t size) {
    return (unsigned) 1 << unsigned (floor(log2(size)));
}

size_t Allocator::findBestFitIndex(size_t requested_memory) {
    size_t free_list_index = free_list_level_limit;
    size_t size = this->min_block_size;

    // if we ever get to index = 0, the allocation will result in nullptr
    while (size < requested_memory && free_list_index != 0) {
        free_list_index--;
        // multiply by 2^1
        size <<= 1;
    }

    return free_list_index;
}

void Allocator::markParentAsSplit(size_t index) {
    index = (index - 1) / 2;
    SplitTable[index / 8] |= (unsigned)1 << (index % 8);
}

void Allocator::unmarkParentAsSplit(size_t index) {
    index = (index - 1) / 2;
    SplitTable[index / 8] &= ~((unsigned)1 << (index % 8));
}

bool Allocator::isSplitBlockByIndex(size_t index) {
    return (SplitTable[index / 8] >> (index % 8)) & 1;
}

bool Allocator::isFreeBlockBuddies(size_t index) {
    index = (index - 1) / 2;
    return (FreeTable[index / 8] >> (index % 8)) & 1;
}

void Allocator::flipFreeTableIndexForBlockBuddies(size_t blockIndex) {
    size_t index = (blockIndex - 1) / 2;
    FreeTable[index / 8] ^= (unsigned)1 << (index % 8);
}

bool Allocator::isSplitByAddrAndLevel(void *adr, size_t level) {
    size_t blockIndex = getBlockIndexFromAddr((uint8_t *)(adr), level);
    return isSplitBlockByIndex(blockIndex);
}

size_t Allocator::findLevelOfAllocatedBlock(void *addr) {
    size_t currentLevel = free_list_level_limit;
    while (currentLevel > 0) {
        // if parent is split, addr is in current level
        if (isSplitByAddrAndLevel(addr, currentLevel - 1)) {
            return currentLevel;
        }
        currentLevel--;
    }
    return 0;
}

size_t Allocator::findBuddyIndex(size_t index) {
    // root node
    if (index == 0) {
        return 0;
    }

    return ((index - 1) ^ 1) + 1;
}

size_t Allocator::previousPowerOfTwo(size_t num) {
    while (num & num - 1) {
        num = num & num -1;
    }

    return num;
}

size_t Allocator::round_up(size_t num, size_t factor) {
    return num + factor - 1 - (num + factor - 1) % factor;
}

bool Allocator::isInnerStructure(size_t index) {
    return ((this->lastInnerStructureBlockIndex - this->overhead_blocks_count) < index) && (index <= this->lastInnerStructureBlockIndex);
}

bool Allocator::isNotAllocated(size_t index, size_t i, void *pVoid) {
    // if parent is not split, there is no way for the current block to be allocated
    if (!this->isSplitBlockByIndex((index - 1) / 2)) {
        return true;
    }
    // if this returns 0, then both the current block and its buddy are allocated
    if (!isFreeBlockBuddies(index)) {
        return false;
    } else {
        // one of the buddies is free, but is it the current one?
        // check if current node is present in free list, if yes -> it is not allocated
        return IsNodePresent(&this->freeLists[i], (Node*) pVoid);
    }
}

Allocator::~Allocator() {
    size_t totalFreeMemory = getCurrentFreeMemory();
    size_t allocatedUserMemory = (this->max_memory_size - totalFreeMemory) - (this->overhead_blocks_count * min_block_size) - (this->actualVirtualSizeDiffRoundedToMinAlloc - this->actualVirtualSizeDiff);
    if (allocatedUserMemory > 0) {
        std::cout << "Destroying Allocator, in spite of memory leak of: " << allocatedUserMemory << " bytes\n\n";
    }
}

size_t Allocator::getCurrentFreeMemory() {
    size_t totalFreeMemory = 0;
    for (int i = 0; i < this->free_list_count; ++i) {
        size_t freeBlockCountOnCurrentLevel = GetLength(&freeLists[i]);
        totalFreeMemory += ((1 << (max_memory_log - i)) * freeBlockCountOnCurrentLevel);
    }

    return totalFreeMemory;
}

Exception::Exception(const char *what) : runtime_error(what) {}
