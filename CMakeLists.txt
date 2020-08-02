cmake_minimum_required(VERSION 3.16)
project(buddy_allocator)

set(CMAKE_CXX_STANDARD 14)

add_executable(buddy_allocator main.cpp Tests.h Tests.cpp BuddyAllocator.h BuddyAllocator.cpp Node.h Node.cpp)