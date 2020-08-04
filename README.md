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
Time taken by buddy to allocate: 2648 microseconds
Time taken by system to allocate: 1775 microseconds
Time taken by buddy to free: 2206 microseconds
Time taken by system to free: 1838 microseconds
```