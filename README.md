# BuddyMemoryAllocator

## Results
### Testing Allocation and Deallocation with 1MB of memory with static 32 byte allocations
```
Time taken by buddy to allocate: 1986 microseconds
Time taken by system to allocate: 1776 microseconds
Time taken by buddy to free: 2204 microseconds
Time taken by system to free: 1758 microseconds
```

### Testing Allocation and Deallocation with 1 GB of memory with ranging allocation sizes
```
Time taken by buddy to allocate: 32263 microseconds
Time taken by system to allocate: 24911 microseconds
Time taken by buddy to free: 28426 microseconds
Time taken by system to free: 26026 microseconds
Ending test huge allocations
```