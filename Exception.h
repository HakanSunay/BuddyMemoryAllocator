//
// Created by Hakan Halil on 9.08.20.
//

#ifndef BUDDY_ALLOCATOR_EXCEPTION_H
#define BUDDY_ALLOCATOR_EXCEPTION_H

#include <stdexcept>

const char* BUDDY_INIT_EXCEPTION_MSG = "Allocator cannot be initialized with size less than twice of minimum block size";
const char* BUDDY_INIT_WITH_NULLPTR_EXCEPTION_MSG = "Allocator cannot be initialized using nullptr as the memory block to be managed";
const char* BUDDY_FREE_EXCEPTION_MSG = "Input address is not managed by the Allocator";

class Exception: public std::runtime_error {
public:
    Exception(const char* what);
};


#endif //BUDDY_ALLOCATOR_EXCEPTION_H
