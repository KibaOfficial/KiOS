// Copyright (c) 2026 KibaOfficial
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "heap.h"
#include "pmm.h"
#include "vmm.h"

// Heap State
static uint64_t heap_current_ptr = HEAP_START;
static uint64_t heap_total_alloc = 0;

/**
 * heap_init - Initialize kernel heap
 */
void heap_init(void) {
    heap_current_ptr = HEAP_START;
    heap_total_alloc = 0;
}

/**
 * kmalloc - Allocate memory from kernel heap
 * @size: Number of bytes to allocate
 *
 * Simple bump allocator. Memory is never freed (kfree is a no-op).
 * Pages are mapped on-demand as the heap grows.
 *
 * Returns: Pointer to allocated memory, or NULL on failure
 */
void* kmalloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    // Align size to 16 bytes for better performance
    size = (size + 15) & ~15;

    // Check if we exceed heap limit
    if (heap_current_ptr + size >= HEAP_START + HEAP_SIZE) {
        return NULL;
    }

    // Calculate which pages we need
    uint64_t start_addr = heap_current_ptr;
    uint64_t end_addr = heap_current_ptr + size;

    uint64_t start_page = start_addr & ~0xFFF;  // Align down to page boundary
    uint64_t end_page = (end_addr + 0xFFF) & ~0xFFF;  // Align up to page boundary

    // Map all required pages
    for (uint64_t page = start_page; page < end_page; page += 0x1000) {
        // Check if page is already mapped
        if (vmm_virt_to_phys(page) == 0) {
            // Not mapped, allocate physical page
            void* phys_page = pmm_alloc_page();
            if (!phys_page) {
                return NULL;
            }

            // Map to virtual address
            vmm_map_page(page, (uint64_t)phys_page, PAGE_PRESENT | PAGE_WRITE);
        }
    }

    // Return pointer and advance heap pointer
    void* ptr = (void*)heap_current_ptr;
    heap_current_ptr += size;
    heap_total_alloc += size;

    return ptr;
}

/**
 * kfree - Free allocated memory (NO-OP for bump allocator)
 * @ptr: Pointer to memory to free
 *
 * This is a dummy function for the bump allocator.
 * Memory is never actually freed until the system reboots.
 *
 * Note: A proper allocator with free-list will be implemented in v0.5.0+
 */
void kfree(void* ptr) {
    // NO-OP for bump allocator
    (void)ptr;
}

/**
 * heap_total_allocated - Get total bytes allocated
 */
uint64_t heap_total_allocated(void) {
    return heap_total_alloc;
}

/**
 * heap_current_size - Get current heap size
 */
uint64_t heap_current_size(void) {
    return heap_current_ptr - HEAP_START;
}
