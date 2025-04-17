#include "utils.h"
#include "types.h"

// Position heap after kernel
#define HEAP_START 0x400000
#define HEAP_SIZE  0x400000
#define HEAP_END   (HEAP_START + HEAP_SIZE)

typedef struct MemBlock {
    uint32 size;
    BOOL used;
    struct MemBlock* next;
} MemBlock;

static MemBlock* heap_start = (MemBlock*)HEAP_START;
static BOOL initialized = FALSE;

void init_heap(void) {
    if (!initialized) {
        heap_start->size = HEAP_SIZE - sizeof(MemBlock);
        heap_start->used = FALSE;
        heap_start->next = NULL;
        initialized = TRUE;
    }
}

void* malloc(uint32 size) {
    if (!initialized) init_heap();
    
    // Align size to 4 bytes
    size = (size + 3) & ~3;
    
    MemBlock* current = heap_start;
    while (current && (uint32)current < HEAP_END) {
        if (!current->used && current->size >= size) {
            if (current->size > size + sizeof(MemBlock) + 8) {
                // Split block if there's enough space for new block plus minimal allocation
                MemBlock* new_block = (MemBlock*)((uint32)current + sizeof(MemBlock) + size);
                new_block->size = current->size - size - sizeof(MemBlock);
                new_block->used = FALSE;
                new_block->next = current->next;
                current->size = size;
                current->next = new_block;
            }
            current->used = TRUE;
            return (void*)((uint32)current + sizeof(MemBlock));
        }
        current = current->next;
    }
    return NULL;
}

void free(void* ptr) {
    if (!ptr || (uint32)ptr < HEAP_START || (uint32)ptr >= HEAP_END) return;
    
    MemBlock* block = (MemBlock*)((uint32)ptr - sizeof(MemBlock));
    if ((uint32)block < HEAP_START || (uint32)block >= HEAP_END) return;
    
    block->used = FALSE;
    
    // Merge with next block if it's free
    while (block->next && 
           (uint32)block->next < HEAP_END && 
           !block->next->used) {
        block->size += sizeof(MemBlock) + block->next->size;
        block->next = block->next->next;
    }
}

int abs(int n) {
    return n < 0 ? -n : n;
}

// Remove memset from utils.c since it's now defined in string.c