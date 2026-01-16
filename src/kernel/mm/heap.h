// Copyright (c) 2026 KibaOfficial
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef KIOS_HEAP_H
#define KIOS_HEAP_H

#include "types.h"

// Kernel Heap Base Address (höhere Hälfte, virtuell)
#define HEAP_START 0xFFFF800000000000ULL
#define HEAP_SIZE  (16 * 1024 * 1024)  // 16MB initial heap size

// Heap Allocator Functions
void heap_init(void);
void* kmalloc(size_t size);
void kfree(void* ptr);

// Heap Statistics
uint64_t heap_total_allocated(void);
uint64_t heap_current_size(void);

#endif /* KIOS_HEAP_H */
