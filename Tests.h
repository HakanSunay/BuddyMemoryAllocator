//
// Created by Hakan Halil on 1.08.20.
//

#ifndef BUDDY_ALLOCATOR_TESTS_H
#define BUDDY_ALLOCATOR_TESTS_H

void StressTestWithAllocateAndFreeAgainstSystem();

void DebugTest();

void MixedTest();

void TestWithPowerOfTwo();

void ProfileAllocator();

void TestHugeAllocations();

void SimpleTest();

void TestSuperSmallAllocator();

void TestFreeInvalidAddress();

void TestAllocateWithSizeMoreThanManaged();

void TestDifferentAllocations();

void TestBiggerBigStructures();

void TestWithCharBufferAlignMissOne();

void TestWithCharBufferAlignMissMoreThanOne();

void TestMisalignedMemoryWithPowerOfTwo();

void TestWithCharBufferWithNonPowTwoSize();


#endif //BUDDY_ALLOCATOR_TESTS_H
