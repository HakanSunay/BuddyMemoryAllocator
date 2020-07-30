#include <iostream>
#include <cmath>

class Node {
public:
    Node * next;
};

Node *Pop(Node **pNode) {
    Node* current = *pNode;
    if (current == nullptr) {
        return nullptr;
    }

    Node* first = current;
    *pNode = current->next;

    return first;
}

void PushNewNode(Node **pNode, Node *pNode1) {
    Node* current = *pNode;
    if (current == nullptr) {
        *pNode = pNode1;
        return;
    }

    while (current && current->next) {
        current = current->next;
    }

    current->next=pNode1;
}

void RemoveNode(Node** pNode, Node* nodeToBeRemoved) {
    Node* cur = *pNode;
    if (cur == nullptr) {
        return;
    }

    if (cur == nodeToBeRemoved) {
        *pNode = nullptr;
        return;
    }

    while (cur->next != nodeToBeRemoved && cur->next != nullptr) {
        cur = cur->next;
    }

    if (cur->next == nullptr) {
        return;
    }

    Node* nextOfNodeToBeRemoved = nodeToBeRemoved->next;
    cur->next = nextOfNodeToBeRemoved;
}

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
    Node** freeLists; // freeList[0] - ; freeList[1] ....

    uint8_t* SplitTable;
    uint8_t* FreeTable;
    size_t TableSize;

    uint8_t *base_ptr;

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
        size_t lastInnerStructureBlockIndex = getBlockIndexFromAddr(base_ptr + (min_block_size * (overhead_blocks_count - 1)), free_list_level_limit);
        size_t currentBlockIndex = lastInnerStructureBlockIndex;
        size_t currentLevel = free_list_level_limit;

        size_t tempIndex = currentBlockIndex;
        for (int i = 0; i < this->overhead_blocks_count; ++i) {
            // markParentAsSplit currently flips the bit, this if ensures it is executed only once for a pair of buddies
            // only when we are at the left buddy will we flip the bit from 0 to 1
            if (tempIndex % 2 == 1) {
                markParentAsSplit(tempIndex);
                unmarkParentAsSplit(tempIndex);
            }
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
            }
            markParentAsSplit(currentBlockIndex);
            currentBlockIndex = getParentIndex(currentBlockIndex);
            currentLevel--;
        }
    }

    Allocator(void* addr, size_t size) {
        base_ptr = (uint8_t *)(addr);

        // TODO: Handle when size is not power of 2
        max_memory_size = size;
        max_memory_log = log2(max_memory_size);

        free_list_count = max_memory_log - min_block_log + 1;
        free_list_level_limit = free_list_count - 1;

        // init free lists
        freeLists = (Node**) addr;
        for (int i = 0; i < free_list_count; ++i) {
            freeLists[i] = ((Node*) addr) + i;
            freeLists[i] = nullptr;
        }

        // HERE: addr + 3 -> NEXT: addr + 4 ( 4 x 8 )
        // Suppose split needs to be SplitTable[2];
        // TODO: I can probably get the last index of freeList and incr it by sizeof(Node*) [8 bytes]
        this->SplitTable = (uint8_t *) (addr) + free_list_count * sizeof(Node *);
        this->TableSize = ((unsigned) 1 << (free_list_count - 1)) / 8;
        for (int j = 0; j < TableSize; ++j) {
            this->SplitTable[j] = 0;
        }

        // TODO: I can probably get the last index of SplitTable and incr it by sizeof(uint8_t) [1 byte]
        this->FreeTable = (u_int8_t *) (addr) + (free_list_count * sizeof(Node *)) + (TableSize * sizeof(uint8_t));
        for (int k = 0; k < TableSize; ++k) {
            this->FreeTable[k] = 0;
        }

        // Must get overhead size
        this->overheadSize = (free_list_count * sizeof(Node *)) + (this->TableSize * sizeof (uint8_t)) + (this->TableSize * sizeof (uint8_t));

        // must find X = overheadSize % min_block_size and mark the first X nodes as used
        // Update SplitTable and freeLists with new values
        this->overhead_blocks_count = ceil(overheadSize / float(min_block_size));

        // updates all necessary inner structures with current state
        initInnerStructures();
    }

    Node *FindRightBuddyOf(void *pVoid, int i) {
        // we have to shift 2^i bytes to the right to find the buddy
        return (Node*)(((char *)pVoid) + (1 << i));
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
        index = (index - 1) / 2;
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
        // Allocate(16)
        // 16, but we need 8 bytes for the header as well => 24 => 32
        //
        // size + HEADER_SIZE = actual_size
        // 0.0. Check if input size is correct
        // 0.1.
        // 1. Closest power of 2 to actual_size ( if actual size 17 -> 32; 5 -> 16) // [HEADER:8BYTE(SIZE_OF_CURRENT_BLOCK) + INPUT_SIZE]
        // 2. Check if free list of level that corresponds to 32 bytes is not empty
        // 2.1 Suppose 32 bytes free list is empty
        // 2.2 Jump to 64 byte free list, divide to two 32s and add them to 32 byte free list
        // 2.3 128


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
            // Current block will becomes free therefore we flip
            flipFreeTableIndexForBlockBuddies(currentIndex);

            // result is BUDDY_IS_FREE XOR 1
            // if 1 / true => buddy is not free
            // if 0 / false => buddy is free
            bool isNotFreeBuddy = isFreeBlockBuddies(currentIndex);
            if (isNotFreeBuddy) {
                // stopping here and will add ourselves to the free lists of our level
                break;
            }

            // we will certainly go 1 level up, so parent will no longer be split
            unmarkParentAsSplit(currentIndex);

            // our buddy is free, therefore we need to remove it from its free list
            // adding to the actual new free list will be done out of the loop
            size_t buddyIndex = findBuddyIndex(currentIndex);
            Node *buddyNode = (Node*)getPtrFromBlockIndex(buddyIndex, currentLevel);
            // TODO: we are going into the free lists, but the buddyNode doesn't exist there
            // We could have made a mistake when marking the free tables
            RemoveNode(&this->freeLists[currentLevel], buddyNode);
            currentIndex = (currentIndex - 1) / 2;
            currentLevel--;
        }

        // hypothetically this should work
        Node* newNode = (Node*)getPtrFromBlockIndex(currentIndex, currentLevel);
        newNode->next = nullptr;
        PushNewNode(&this->freeLists[currentLevel], newNode);
    }
};

int main() {
    void *adr = malloc(512);

    Allocator a  = Allocator(adr, 256);

    // TODO: Stress testing with bigger allocator size and different allocation sizes
    // Possible regression might happen in FreeTable values, but debugging will be hard
    int *nums[3];
    for (int i = 0; i < 3; ++i) {
        nums[i] = (int*)a.Allocate(sizeof(int));
        *nums[i] = i;
    }

    for (int i = 0; i < 3; ++i) {
        if (*nums[i] != i) {
            printf("OPA!!!");
        }
    }

    a.Free(nums[2]);
    a.Free(nums[1]);

    std::cout << "Hello, World!" << std::endl;
    free(adr);
    return 0;
}
