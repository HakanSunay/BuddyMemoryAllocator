//
// Created by Hakan Halil on 1.08.20.
//

#ifndef UNTITLED_NEWEST_NODE_H
#define UNTITLED_NEWEST_NODE_H

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

#endif //UNTITLED_NEWEST_NODE_H