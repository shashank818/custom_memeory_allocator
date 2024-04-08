#include <stdio.h>
#include <unistd.h> // For sbrk
#include <stdbool.h>
#include <string.h> // For memcpy
 
// Define a block structure to manage memory
typedef struct block {
    size_t size;
    bool is_free;
    struct block *next;
} block_t;
 
#define BLOCK_SIZE sizeof(block_t)
 
void *base = NULL; // Start of our memory pool
 
// Find a free block that fits the requested size
block_t *find_free_block(block_t **last, size_t size) {
    block_t *current = base;
    while (current && !(current->is_free && current->size >= size)) {
        *last = current;
        current = current->next;
    }
    return current;
}
 
// Request more memory from the system
block_t request_space(block_t last, size_t size) {
    block_t *block = sbrk(0);
    void *request = sbrk(size + BLOCK_SIZE);
    if (request == (void*) -1) {
        return NULL; // sbrk failed
    }
 
    if (last) { // NULL on first request
        last->next = block;
    }
 
    block->size = size;
    block->is_free = false;
    block->next = NULL;
    return block;
}
 
void *malloc(size_t size) {
    if (size <= 0) {
        return NULL;
    }
 
    block_t *block;
    if (!base) { // First call
        block = request_space(NULL, size);
        if (!block) {
            return NULL;
        }
        base = block;
    } else {
        block_t *last = base;
        block = find_free_block(&last, size);
        if (!block) { // Failed to find free block
            block = request_space(last, size);
            if (!block) {
                return NULL;
            }
        } else { // Found free block
            block->is_free = false;
        }
    }
 
    return (block+1); // Return a pointer to the space right after the block structure
}
 
void free(void *ptr) {
    if (!ptr) {
        return;
    }
 
    // Get the block pointer from the user pointer
    block_t block_ptr = (block_t)ptr - 1;
    block_ptr->is_free = true;
    // TODO: Coalesce free blocks here if you want to reduce fragmentation
}
 
int main() {
    printf("Custom memory allocator example\n");
 
    int num = (int)malloc(sizeof(int));
    if (num == NULL) {
        printf("Failed to allocate memory\n");
        return 1;
    }
    *num = 123;
    printf("Number = %d\n", *num);
    free(num);
 
    return 0;
}