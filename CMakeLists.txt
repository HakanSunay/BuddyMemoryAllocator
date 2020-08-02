cmake_minimum_required(VERSION 3.16)
project(untitled_newest)

set(CMAKE_CXX_STANDARD 14)

add_executable(untitled_newest main.cpp Tests.h Tests.cpp BuddyAllocator.h BuddyAllocator.cpp Node.h Node.cpp)