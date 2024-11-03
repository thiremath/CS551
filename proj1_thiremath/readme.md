# Name- Tejas Hiremath

# B01038537

# Status- Complete

# Project1- Systems Programming

# Memory Manager Project

## Overview

This project implements a memory manager for efficient memory allocation and deallocation. It uses fixed-size memory slots and supports dynamic memory allocation with optimized batch-based organization. The memory manager prevents fragmentation by using bitmaps to track available slots within memory batches and provides error handling for invalid memory operations like double freeing, freeing invalid pointers, and out-of-range pointers.

## Features

- Fixed Slot Allocation: Memory is allocated in fixed-size slots to maintain efficiency.
- Batch Management: The memory manager groups memory into batches, allowing seamless expansion as required.
- Bitmap Tracking: A bitmap tracks each slot's usage status within a batch, reducing fragmentation.
- Error Handling: The manager identifies invalid memory operations, including double-free attempts, non-slot start addresses, and out-of-range pointers.

## Getting Started

### Prerequisites

This project requires the following tools:
- GCC compiler (or any compatible C compiler)
- Make (for build automation)

### Project Structure

├── memory_manager.h         # Header file with declarations
├── memory_manager.c         # Implementation of the memory manager
├── test_main.c              # Main test file with test cases
├── Makefile                 # Makefile for building the project
└── README.md                # Project documentation

### Compilation

To build the project, run the following command:

make

This will compile memory_manager.c and test_main.c, and create the executable test_main.

### Running the Tests

To execute the test suite, run:

./test_main

The output will display the allocation and deallocation sequence along with memory snapshots.

## Code Explanation

### Core Functions

1. mem_mngr_init(): Initializes the memory manager, creating a default memory list with a single batch of 16-byte slots.

2. mem_mngr_alloc(size_t size): Allocates a memory slot of at least the specified size. The function searches for a free slot in the appropriate list or creates a new batch when the list is full.

3. mem_mngr_free(void *ptr): Frees the specified memory slot and marks it as available. It also includes checks to prevent double-free and invalid address errors.

4. mem_mngr_print_snapshot(): Displays the current state of the memory manager, including slot status and batch information.

5. mem_mngr_leave(): Cleans up and releases all allocated memory, clearing the memory pool.

### Helper Functions

- create_memory_list(int slot_size): Creates a new memory list for a specified slot size and initializes a batch.
- find_or_create_list(int slot_size): Locates an existing list that can handle the specified slot size or creates a new one.

### Bitmap Utilities

- bitmap_find_first_bit(): Finds the first available slot (bit) in a bitmap.
- bitmap_set_bit() and bitmap_clear_bit(): Sets or clears bits to indicate slot availability.

### Error Handling

The project includes error handling for invalid memory operations:

- Double Free: Attempts to free a slot that has already been freed.
- Invalid Slot Address: Attempts to free an address that is not the start of a slot.
- Out-of-Range Pointer: Attempts to free a pointer outside of any managed memory block.

## Example Output

Sample output is included to illustrate bitmap updates, allocation, and deallocation:


Test (a): Verify Bitmap Updates During Memory Allocation
Allocated 16 bytes at: 0x152e05cf0
...
============== Memory snapshot ===============
mem_list 0x152e06040 slot_size 16 batch_count 2 free_slot_bitmap 0x152e05ef0
bitmap 0x152e05ef0 size 2 bytes: 0000  0000  0111  1111  



# Log of Output(All Test Cases Working):

Test (a): Verify Bitmap Updates During Memory Allocation
Allocated 16 bytes at: 0x15b9310
Allocated 16 bytes at: 0x15b9320
Allocated 16 bytes at: 0x15b9330
Allocated 16 bytes at: 0x15b9340
Allocated 16 bytes at: 0x15b9350
Allocated 16 bytes at: 0x15b9360
Allocated 16 bytes at: 0x15b9370
Allocated 16 bytes at: 0x15b9380
============== Memory snapshot ===============
mem_list 0x15b92a0 slot_size 16 batch_count 1 free_slot_bitmap 0x15b92d0
bitmap 0x15b92d0 size 1 bytes: 0000  0000  
     mem_batch 0x15b92f0 batch_mem 0x15b9310
==============================================

Test (b): Handle Allocation Requests Exceeding Current Batch Capacity
Allocated extra 16 bytes at (should be in a new batch): 0x15b97d0
============== Memory snapshot ===============
mem_list 0x15b92a0 slot_size 16 batch_count 2 free_slot_bitmap 0x15b9860
bitmap 0x15b9860 size 2 bytes: 0000  0000  0111  1111  
     mem_batch 0x15b92f0 batch_mem 0x15b9310
     mem_batch 0x15b97b0 batch_mem 0x15b97d0
==============================================

Test (c): Validate Free Operation for Expected and Unexpected Cases
============== Memory snapshot ===============
mem_list 0x15b92a0 slot_size 16 batch_count 2 free_slot_bitmap 0x15b9860
bitmap 0x15b9860 size 2 bytes: 1111  1111  1111  1111  
     mem_batch 0x15b92f0 batch_mem 0x15b9310
     mem_batch 0x15b97b0 batch_mem 0x15b97d0
==============================================
Allocated 16 bytes at: 0x15b9310
Freeing allocated block (expected case).
============== Memory snapshot ===============
mem_list 0x15b92a0 slot_size 16 batch_count 2 free_slot_bitmap 0x15b9860
bitmap 0x15b9860 size 2 bytes: 0111  1111  1111  1111  
     mem_batch 0x15b92f0 batch_mem 0x15b9310
     mem_batch 0x15b97b0 batch_mem 0x15b97d0
==============================================
Attempting double free (should produce an error).
Error: ptr is the starting address of an unassigned slot - double freeing
Attempting to free a non-block start address (should produce an error).
Error: ptr is not the starting address of any slot
Attempting to free an out-of-range pointer (should produce an error).
Error: ptr is outside of memory managed by the manager

Test (d): Handle Larger Memory Allocations with New Batches
Allocated 32 bytes at: 0x15b98d0
============== Memory snapshot ===============
mem_list 0x15b92a0 slot_size 16 batch_count 2 free_slot_bitmap 0x15b9860
bitmap 0x15b9860 size 2 bytes: 1111  1111  1111  1111  
     mem_batch 0x15b92f0 batch_mem 0x15b9310
     mem_batch 0x15b97b0 batch_mem 0x15b97d0
mem_list 0x15b9880 slot_size 32 batch_count 1 free_slot_bitmap 0x15b92d0
bitmap 0x15b92d0 size 1 bytes: 0111  1111  
     mem_batch 0x15b98b0 batch_mem 0x15b98d0
==============================================


## Troubleshooting

1. Memory Not Freed: Ensure proper alignment and accurate slot tracking in the bitmap. Verify that 'bitmap_clear_bit' correctly identifies and marks slots.
2. Double-Free Errors: Confirm that all pointers to freed slots are reset or managed to prevent unintended operations.