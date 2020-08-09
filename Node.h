//
// Created by Hakan Halil on 1.08.20.
//

#ifndef BUDDY_ALLOCATOR_NODE_H
#define BUDDY_ALLOCATOR_NODE_H

#include <cstdlib>

class Node {
public:
    Node * next;
};

Node *Pop(Node **pNode);

void PushNewNode(Node **pNode, Node *pNode1);

void RemoveNode(Node** pNode, Node* nodeToBeRemoved);

bool IsNodePresent(Node** pNode, Node* checkNode);

size_t GetLength(Node** list);

#endif //BUDDY_ALLOCATOR_NODE_H