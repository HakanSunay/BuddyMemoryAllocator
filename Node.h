//
// Created by Hakan Halil on 1.08.20.
//

#ifndef UNTITLED_NEWEST_NODE_H
#define UNTITLED_NEWEST_NODE_H

#include <cstdlib>

class Node {
public:
    Node * next;
};

Node *Pop(Node **pNode);

void PushNewNode(Node **pNode, Node *pNode1);

void RemoveNode(Node** pNode, Node* nodeToBeRemoved);

size_t GetLength(Node** list);

#endif //UNTITLED_NEWEST_NODE_H