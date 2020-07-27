#include <iostream>
#include <cmath>

class Node {
public:
    Node * next;
};

class LinkedList {

public:
    Node* head;

    LinkedList() {
        head = nullptr;
    }

    void Push(Node* newNode) {
        if (head == nullptr) {
            head = newNode;
            return;
        }

        Node* current = this->head;

        while (current && current->next) {
            current = current->next;
        }

        current->next=newNode;
    }

    Node* PopFront() {
        if (head == nullptr) {
            return nullptr;
        }

        Node* first = this->head;
        this->head = this->head->next;

        return first;
    }
};

class Allocator {
    LinkedList** freeLists; // freeList[0] - ; freeList[1] ....

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
        // if we have filled uneven amount of leaf nodes, the last leaf node must join the free list
        if (count % 2 == 1) {
            size_t offset = count * min_block_size;


            // TODO: This must be done for every level, what happens when this block is allocated? Currently 1 block (16byte) is used for the LL and the NewNode
            // Suppose we have BL 1 -> BL 2 -> BL 3
            // BL 1 has the LinkedList that is pointed to by freeLists[i] and the head Node
            // BL 2 is pointed to by the head node and has its next node
            // BL 3 is pointed to by the BL 2 next node and its next is NULL
            // What happens if we need to give BL1 for allocation?
            // We have to remove the head (no problem)
            // BUT WE ALSO HAVE TO REMOVE THE LL ITSELF, WE NEED TO MOVE IT NEXT TO BL2
            // or in other words push BL2 a couple of bytes to the right so as to make space for the linked list
            // See if we can actually store the actual linked list in the last BL?
            // Does this mean that Push operation must always move the linked list to the new memory block?
            // CAN THIS BE RESOLVED BY REMOVING THE LINKED LIST ABSTRACTION AND USING THE NODE* ONLY?

            // LL is not init, here we are initing it
            LinkedList* ll =((LinkedList *)((char*)base_ptr + offset));
            ll->head = nullptr;

            offset += sizeof(LinkedList);
            Node *newNode = (Node*)((char *)base_ptr + offset);
            newNode->next = nullptr;
            ll->Push(newNode);

            freeLists[free_list_count - 1] = ll;
            if (true) {
                int a = 5 + 4;
            }
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
        freeLists = (LinkedList**) addr;
        for (int i = 0; i < free_list_count; ++i) {
            freeLists[i] = ((LinkedList*) addr) + i;
            freeLists[i] = nullptr;
        }

        // HERE: addr + 3 -> NEXT: addr + 4 ( 4 x 8 )
        // Suppose split needs to be isSplit[2];
        this->isSplit = (uint8_t *) (addr) + free_list_count * sizeof(LinkedList *);
        this->isSplitCount = ((unsigned) 1 << (free_list_count - 1)) / 8;
        for (int j = 0; j < isSplitCount; ++j) {
            this->isSplit[j] = 0;
        }

        // Must get overhead size
        this->overheadSize = (free_list_count * sizeof(LinkedList *)) + (this->isSplitCount * sizeof (uint8_t));

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
