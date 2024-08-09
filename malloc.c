// Inspired by https://github.com/danluu/malloc-tutorial

#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
// Don't include stdlib since the names will conflict?

// TODO: align memory allocation properly

// Allocate memory without the ability to free or reallocate.
// Uses sbrk to increase program data space.
void *simple_malloc(size_t size) {
    // Get the current program break (end of the process's data segment)
    void *current_break = sbrk(0); 
    // Increase the program break by 'size' bytes
    void *new_break = sbrk(size);  
    
    // Check if sbrk failed (returns (void *) -1 on failure)
    if (new_break == (void *) -1) {
        return NULL; // Allocation failed
    }

    // Ensure the allocation is contiguous (not thread-safe)
    assert(current_break == new_break); 
    // Return the start of the allocated memory
    return current_break; 
}

// Metadata structure for managing memory blocks
struct mem_block {
    size_t size;             // Size of the block
    struct mem_block *next;  // Pointer to the next block in the list
    int free;                // Flag indicating if the block is free
    int debug_flag;          // Debugging flag, remove in production
};

// Size of the metadata structure
#define BLOCK_SIZE sizeof(struct mem_block)

// Base of the memory block list
void *base = NULL;

// Find a free block of sufficient size
// TODO: split blocks if they are larger than necessary
struct mem_block *locate_free_block(struct mem_block **last, size_t size) {
    struct mem_block *current = base;
    // Iterate through the list of blocks
    while (current && !(current->free && current->size >= size)) {
        *last = current;       // Update the last pointer
        current = current->next; // Move to the next block
    }
    return current; // Return the found block or NULL if not found
}

// Extend the program data space by allocating a new block
struct mem_block *extend_memory(struct mem_block* last, size_t size) {
    struct mem_block *block;
    block = sbrk(0); // Get the current program break
    void *new_break = sbrk(size + BLOCK_SIZE); // Increase program break by block size + metadata size
    
    // Check if sbrk failed
    if (new_break == (void *) -1) {
        return NULL;
    }

    // Ensure the allocation is contiguous (not thread-safe)
    assert((void *)block == new_break);

    // Update the last block's next pointer if it exists
    if (last) {
        last->next = block;
    }
    
    // Initialize the new block's metadata
    block->size = size;
    block->next = NULL;
    block->free = 0;
    block->debug_flag = 0x12345678; // Debugging flag, can be removed in production
    return block;
}

// Allocate memory of given size
void *allocate_memory(size_t size) {
    struct mem_block *block;
    
    // Validate size
    if (size <= 0) {
        return NULL;
    }

    // First allocation
    if (!base) {
        block = extend_memory(NULL, size); // Request new space
        if (!block) {
            return NULL;
        }
        base = block; // Set the base to the first block
    } else {
        struct mem_block *last = base;
        block = locate_free_block(&last, size); // Try to find a free block
        
        // No suitable free block found, extend memory
        if (!block) {
            block = extend_memory(last, size); // Request new space
            if (!block) {
                return NULL;
            }
        } else {
            // Suitable block found, mark as used
            block->free = 0;
            block->debug_flag = 0x77777777; // Debugging flag
        }
    }
    
    return (block + 1); // Return the address after the block metadata
}

// Allocate memory and set it to zero
void *zero_allocate(size_t elements, size_t element_size) {
    size_t total_size = elements * element_size;
    void *ptr = allocate_memory(total_size); // Allocate memory
    if (ptr) {
        memset(ptr, 0, total_size); // Set allocated memory to zero
    }
    return ptr;
}

// Get the block metadata from a pointer
struct mem_block *retrieve_block_ptr(void *ptr) {
    return (struct mem_block *)ptr - 1; // Adjust the pointer to get metadata
}

// Free the allocated memory
void release_memory(void *ptr) {
    if (!ptr) {
        return;
    }

    struct mem_block* block_ptr = retrieve_block_ptr(ptr); // Get block metadata
    assert(block_ptr->free == 0); // Ensure the block is not already free
    assert(block_ptr->debug_flag == 0x77777777 || block_ptr->debug_flag == 0x12345678); // Check debug flag
    block_ptr->free = 1; // Mark the block as free
    block_ptr->debug_flag = 0x55555555; // Update debug flag
}

// Reallocate memory
void *reallocate_memory(void *ptr, size_t size) {
    if (!ptr) {
        return allocate_memory(size); // Act like malloc
    }

    struct mem_block* block_ptr = retrieve_block_ptr(ptr); // Get block metadata
    
    // Current block is sufficient
    if (block_ptr->size >= size) {
        return ptr;
    }

    // Allocate new block and copy data
    void *new_ptr = allocate_memory(size);
    if (!new_ptr) {
        return NULL;
    }

    memcpy(new_ptr, ptr, block_ptr->size); // Copy old data to new block
    release_memory(ptr); // Free old block
    return new_ptr;
}

// Main function to demonstrate memory allocation
int main() {
    // Allocate 20 bytes of memory
    char *ptr1 = (char *) allocate_memory(20);
    strcpy(ptr1, "Hello, World!"); // Use the allocated memory
    printf("%s\n", ptr1); // Output: Hello, World!

    // Allocate and zero-initialize an array
    int *ptr2 = (int *) zero_allocate(5, sizeof(int)); // Allocate array of 5 integers
    for (int i = 0; i < 5; i++) {
        printf("%d ", ptr2[i]); // Output: 0 0 0 0 0
    }
    printf("\n");

    // Reallocate memory
    char *ptr3 = (char *) reallocate_memory(ptr1, 40);
    strcat(ptr3, " Welcome to memory management!"); // Use the reallocated memory
    printf("%s\n", ptr3); // Output: Hello, World! Welcome to memory management!

    // Free allocated memory
    release_memory(ptr2);
    release_memory(ptr3);

    return 0;
}
