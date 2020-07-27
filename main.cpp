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
        current = pNode1;
        return;
    }

    while (current && current->next) {
        current = current->next;
    }

    current->next=pNode1;
}

class Allocator {
    Node** freeLists; // freeList[0] - ; freeList[1] ....

    uint8_t* isSplit;
    size_t isSplitCount;

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
    void addNMinimumSizedBlocks(size_t count) {
        // TODO: Configure all inner structure data, mark as split and so on
        // TODO: Initialize all of the necessary free lists with the correct addresses

        // TODO: There must be a way to achieve this recursively or depending on the count and the sizes we have, we must be able to set everything
        // if we have filled uneven amount of leaf nodes, the last leaf node must join the free list
        if (count % 2 == 1) {
            size_t offset = count * min_block_size;

            Node* ll =((Node *)((char*)base_ptr + offset));
            ll->next = nullptr;

            PushNewNode(&freeLists[4], ll);
            freeLists[free_list_count - 1] = ll;
        }
    }

// TODO: How to order inner structures' data after they are initialized
    // TODO: Preamble vs Other ways for size persistence
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
        // Suppose split needs to be isSplit[2];
        this->isSplit = (uint8_t *) (addr) + free_list_count * sizeof(Node *);
        this->isSplitCount = ((unsigned) 1 << (free_list_count - 1)) / 8;
        for (int j = 0; j < isSplitCount; ++j) {
            this->isSplit[j] = 0;
        }

        // Must get overhead size
        this->overheadSize = (free_list_count * sizeof(Node *)) + (this->isSplitCount * sizeof (uint8_t));

        // must find X = overheadSize % min_block_size and mark the first X nodes as used
        // Update isSplit and freeLists with new values
        this->overhead_blocks_count = ceil(overheadSize / float(min_block_size));

        // updates all necessary inner structures with current state
        addNMinimumSizedBlocks(this->overhead_blocks_count);
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
        return nullptr;
    }

    void Free(void* ptr) {
        // SIZE_OF_BLOCK = (size_t) ptr - 1;
        //
    }
};

int main() {

    void *adr = malloc(512);

    Allocator(adr, 256);

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
