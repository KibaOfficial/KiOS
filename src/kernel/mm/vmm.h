// Copyright (c) 2026 KibaOfficial
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "types.h"

// Page Table Entry Flags
#define PAGE_PRESENT   (1ULL << 0)   // Page ist gemapped
#define PAGE_WRITE     (1ULL << 1)   // Schreibbar
#define PAGE_USER      (1ULL << 2)   // User-Mode Zugriff erlaubt
#define PAGE_NOCACHE   (1ULL << 4)   // Kein Cache (für MMIO)
#define PAGE_SIZE_2MB  (1ULL << 7)   // 2MB Page (für PD Entries)

// Page Size
#define PAGE_SIZE 4096

// Page Table Indices aus virtueller Adresse extrahieren
#define PML4_INDEX(addr) (((addr) >> 39) & 0x1FF)
#define PDPT_INDEX(addr) (((addr) >> 30) & 0x1FF)
#define PD_INDEX(addr)   (((addr) >> 21) & 0x1FF)
#define PT_INDEX(addr)   (((addr) >> 12) & 0x1FF)

// Page Table Entry Struktur
typedef uint64_t pte_t;

// VMM Functions
void vmm_init(void);
void vmm_map_page(uint64_t virt_addr, uint64_t phys_addr, uint32_t flags);
void vmm_unmap_page(uint64_t virt_addr);
uint64_t vmm_virt_to_phys(uint64_t virt_addr);

// Helper: Hole aktuelles CR3 (PML4 Physical Address)
static inline uint64_t vmm_get_cr3(void) {
    uint64_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

// Helper: Setze CR3 (lade neue Page Tables)
static inline void vmm_set_cr3(uint64_t cr3) {
    __asm__ volatile("mov %0, %%cr3" :: "r"(cr3) : "memory");
}

// Helper: TLB flush für eine Adresse
static inline void vmm_invlpg(uint64_t addr) {
    __asm__ volatile("invlpg (%0)" :: "r"(addr) : "memory");
}
