# Simple Memory Allocator

This project implements a basic memory allocator inspired by [danluu's malloc-tutorial](https://github.com/danluu/malloc-tutorial). The code demonstrates how to allocate, reallocate, and free memory using system calls like `sbrk`. This allocator is a minimalistic example and is not intended for production use due to its lack of thread safety, alignment considerations, and other optimizations.

## Features

- **Memory Allocation:** Allocate memory blocks using `sbrk` with `simple_malloc`.
- **Memory Management:** 
  - `allocate_memory`: Allocates a memory block of the specified size.
  - `zero_allocate`: Allocates memory and initializes it to zero.
  - `reallocate_memory`: Reallocates memory to a new size while preserving existing data.
  - `release_memory`: Frees allocated memory blocks.
- **Debugging Flags:** Simple debugging flags are used to track memory block states.

## Usage

The code includes a `main` function that demonstrates basic usage of the memory allocator:

- Allocating and using memory.
- Zero-initializing an array.
- Reallocating memory to a larger size.
- Freeing memory blocks.

## Building and Running

To compile and run the program:

```bash
gcc -o simple_malloc simple_malloc.c
./simple_malloc
