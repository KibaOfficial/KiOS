#include "types.h"
/**
 * Copyright (c) 2026 KibaOfficial
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include "types.h"

extern uint8_t __kernel_start;
extern uint8_t __kernel_end;

#include "pmm.h"
#include "memory_map.h"
#include "vga.h"

#define PAGE_SIZE 4096

static uint8_t *bitmap;
static uint64_t total_pages;
static uint64_t used_pages;

void pmm_init(void)
{
	uint16_t count = memory_map_entry_count();
	memory_map_entry_t *entries = memory_map_entries();

	/* maximale physische Adresse bestimmen */
	uint64_t max_addr = 0;
	for (uint16_t i = 0; i < count; i++)
	{
		uint64_t end = entries[i].base + entries[i].length;
		if (end > max_addr)
		{
			max_addr = end;
		}
	}

	total_pages = max_addr / PAGE_SIZE;

	/* Bitmap nach Kernel Ende */
	uint64_t bitmap_size = (total_pages + 7) / 8;
	bitmap = (uint8_t *)(((uint64_t)&__kernel_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1));

	/* Alles als belegt markieren */
	for (uint64_t i = 0; i < bitmap_size; i++)
	{
		bitmap[i] = 0xFF;
	}
	used_pages = total_pages;

	for (uint16_t i = 0; i < count; i++)
	{
		if (entries[i].type != 1)
			continue;

		uint64_t start = entries[i].base / PAGE_SIZE;
		uint64_t pages = entries[i].length / PAGE_SIZE;

		for (uint64_t p = 0; p < pages; p++)
		{
			uint64_t idx = start + p;
			uint64_t byte = idx / 8;
			uint8_t bit = 1 << (idx % 8);

			if (bitmap[byte] & bit)
			{
				bitmap[byte] &= ~bit;
				used_pages--;
			}
		}
	}

	/* Kernel und Bitmap reservieren */
	uint64_t kernel_start = (uint64_t)&__kernel_start;
	uint64_t kernel_end = (uint64_t)bitmap + bitmap_size;
	uint64_t start_page = kernel_start / PAGE_SIZE;
	uint64_t end_page = (kernel_end + PAGE_SIZE - 1) / PAGE_SIZE;

	for (uint64_t p = start_page; p < end_page; p++)
	{
		uint64_t byte = p / 8;
		uint8_t bit = 1 << (p % 8);
		if (!(bitmap[byte] & bit))
		{
			bitmap[byte] |= bit;
			used_pages++;
		}
	}

	vga_println("[PMM] Initialized Physical Memory Manager");
}

void* pmm_alloc_page(void) {
    for (uint64_t i = 0; i < total_pages; i++) {
        uint64_t byte = i / 8;
        uint8_t  bit  = 1 << (i % 8);
        if (!(bitmap[byte] & bit)) {
            bitmap[byte] |= bit;
            used_pages++;
            return (void*)(i * PAGE_SIZE);
        }
    }
    return 0; // Kein Speicher frei
}

void pmm_free_page(void* phys) {
	uint64_t page = (uint64_t)phys / PAGE_SIZE;
	uint64_t byte = page / 8;
	uint8_t  bit  = 1 << (page % 8);
	if (bitmap[byte] & bit) {
		bitmap[byte] &= ~bit;
		used_pages--;
	}
}

uint64_t pmm_total_pages(void) {
	return total_pages;
}

uint64_t pmm_used_pages(void) {
	return used_pages;
}