//
// Created by Hakan Halil on 1.08.20.
//

#include <iostream>
#include "Node.h"

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
        if (current == current ->next) {
            break;
        }
        current = current->next;
    }

    current->next=pNode1;
}

void RemoveNode(Node **pNode, Node *nodeToBeRemoved) {
    Node* cur = *pNode;
    if (cur == nullptr) {
        return;
    }

    if (cur == nodeToBeRemoved) {
        *pNode = cur->next;
        return;
    }

    while (cur->next != nodeToBeRemoved && cur->next != nullptr) {
        cur = cur->next;
    }

    if (cur->next == nullptr) {
        std::cout<<"MISS"<<std::endl;
        return;
    }

    Node* nextOfNodeToBeRemoved = nodeToBeRemoved->next;
    cur->next = nextOfNodeToBeRemoved;
}

size_t GetLength(Node **list) {
    Node* cur = *list;
    if (cur == nullptr) {
        return 0;
    }

    size_t len = 0;
    while (cur) {
        cur = cur->next;
        len++;
    }

    return len;
}

bool IsNodePresent(Node **pNode, Node *checkNode) {
    Node* cur = *pNode;
    if (cur == nullptr) {
        return false;
    }

    while (cur) {
        if (cur == checkNode) {
            return true;
        }
        cur = cur->next;
    }

    return false;
}
