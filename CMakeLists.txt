cmake_minimum_required(VERSION 3.16)
project(buddy_allocator)

set(CMAKE_CXX_STANDARD 14)

add_executable(buddy_allocator main.cpp Tests.h Tests.cpp BuddyAllocator.h BuddyAllocator.cpp Node.h Node.cpp)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(buddy_allocator PUBLIC DEBUG=1)
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    target_compile_definitions(buddy_allocator PUBLIC DEBUG=0)
endif()