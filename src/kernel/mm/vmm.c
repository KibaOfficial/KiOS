// Copyright (c) 2026 KibaOfficial
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "vmm.h"
#include "pmm.h"
#include "vga.h"

// Aktuelles PML4 (von stage2 erstellt)
static uint64_t current_pml4_phys;

void vmm_init(void) {
    // Hole aktuelles CR3 (zeigt auf PML4 von Bootloader)
    current_pml4_phys = vmm_get_cr3();

    // VMM initialisiert - keine Ausgabe für sauberes Boot
}

// Helper: Hole Page Table Entry Pointer (erstellt Tables bei Bedarf)
// flags werden an die Parent-Entries weitergegeben (für PAGE_USER)
static pte_t* vmm_get_pte_flags(uint64_t virt_addr, int create, uint32_t flags) {
    // Parent flags: PAGE_USER muss in alle Levels propagiert werden!
    uint64_t parent_flags = PAGE_PRESENT | PAGE_WRITE;
    if (flags & PAGE_USER) {
        parent_flags |= PAGE_USER;
    }

    // PML4 Entry
    pte_t* pml4 = (pte_t*)current_pml4_phys;
    uint64_t pml4_idx = PML4_INDEX(virt_addr);

    if (!(pml4[pml4_idx] & PAGE_PRESENT)) {
        if (!create) return 0;

        // Alloziere neue PDPT
        uint64_t pdpt_phys = (uint64_t)pmm_alloc_page();
        if (!pdpt_phys) return 0;

        // PDPT mit Nullen initialisieren
        pte_t* pdpt = (pte_t*)pdpt_phys;
        for (int i = 0; i < 512; i++) {
            pdpt[i] = 0;
        }

        // Memory Barrier: Sicherstellen dass alle Writes abgeschlossen sind
        __asm__ volatile("mfence" ::: "memory");

        // PML4 Entry setzen MIT parent_flags (inkl. PAGE_USER wenn nötig)
        pml4[pml4_idx] = pdpt_phys | parent_flags;

        // Memory Barrier nach Parent Entry Write
        __asm__ volatile("mfence" ::: "memory");
    } else if ((flags & PAGE_USER) && !(pml4[pml4_idx] & PAGE_USER)) {
        // Entry existiert aber hat kein USER bit - hinzufügen!
        pml4[pml4_idx] |= PAGE_USER;
        __asm__ volatile("mfence" ::: "memory");
    }

    // PDPT Entry
    pte_t* pdpt = (pte_t*)(pml4[pml4_idx] & ~0xFFF);
    uint64_t pdpt_idx = PDPT_INDEX(virt_addr);

    if (!(pdpt[pdpt_idx] & PAGE_PRESENT)) {
        if (!create) return 0;

        // Alloziere neue PD
        uint64_t pd_phys = (uint64_t)pmm_alloc_page();
        if (!pd_phys) return 0;

        // PD mit Nullen initialisieren
        pte_t* pd = (pte_t*)pd_phys;
        for (int i = 0; i < 512; i++) {
            pd[i] = 0;
        }

        // Memory Barrier
        __asm__ volatile("mfence" ::: "memory");

        // PDPT Entry setzen MIT parent_flags
        pdpt[pdpt_idx] = pd_phys | parent_flags;

        // Memory Barrier
        __asm__ volatile("mfence" ::: "memory");
    } else if ((flags & PAGE_USER) && !(pdpt[pdpt_idx] & PAGE_USER)) {
        // Entry existiert aber hat kein USER bit - hinzufügen!
        pdpt[pdpt_idx] |= PAGE_USER;
        __asm__ volatile("mfence" ::: "memory");
    }

    // PD Entry
    pte_t* pd = (pte_t*)(pdpt[pdpt_idx] & ~0xFFF);
    uint64_t pd_idx = PD_INDEX(virt_addr);

    if (!(pd[pd_idx] & PAGE_PRESENT)) {
        if (!create) return 0;

        // Alloziere neue PT
        uint64_t pt_phys = (uint64_t)pmm_alloc_page();
        if (!pt_phys) return 0;

        // PT mit Nullen initialisieren
        pte_t* pt = (pte_t*)pt_phys;
        for (int i = 0; i < 512; i++) {
            pt[i] = 0;
        }

        // Memory Barrier
        __asm__ volatile("mfence" ::: "memory");

        // PD Entry setzen MIT parent_flags
        pd[pd_idx] = pt_phys | parent_flags;

        // Memory Barrier
        __asm__ volatile("mfence" ::: "memory");
    } else if ((flags & PAGE_USER) && !(pd[pd_idx] & PAGE_USER)) {
        // Entry existiert aber hat kein USER bit - hinzufügen!
        pd[pd_idx] |= PAGE_USER;
        __asm__ volatile("mfence" ::: "memory");
    }

    // PT Entry
    pte_t* pt = (pte_t*)(pd[pd_idx] & ~0xFFF);
    uint64_t pt_idx = PT_INDEX(virt_addr);

    return &pt[pt_idx];
}

// Wrapper für Rückwärtskompatibilität (ohne flags)
static pte_t* vmm_get_pte(uint64_t virt_addr, int create) {
    return vmm_get_pte_flags(virt_addr, create, 0);
}

void vmm_map_page(uint64_t virt_addr, uint64_t phys_addr, uint32_t flags) {
    // Hier MUSS flags übergeben werden, damit pte_get_flags das USER-Bit sieht!
    pte_t* pte = vmm_get_pte_flags(virt_addr, 1, flags);
    
    if (!pte) {
        vga_println("[VMM] ERROR: Failed to get PTE!");
        return;
    }

    *pte = (phys_addr & ~0xFFF) | flags | PAGE_PRESENT;

    __asm__ volatile("mfence" ::: "memory");
    vmm_invlpg(virt_addr);
}

void vmm_unmap_page(uint64_t virt_addr) {
    pte_t* pte = vmm_get_pte(virt_addr, 0);
    if (!pte || !(*pte & PAGE_PRESENT)) {
        return; // Nicht gemapped
    }

    // Entry löschen
    *pte = 0;

    // Memory Barrier
    __asm__ volatile("mfence" ::: "memory");

    // TLB invalidieren
    vmm_invlpg(virt_addr);
}

uint64_t vmm_virt_to_phys(uint64_t virt_addr) {
    pte_t* pte = vmm_get_pte(virt_addr, 0);
    if (!pte || !(*pte & PAGE_PRESENT)) {
        return 0; // Nicht gemapped
    }

    // Physical Address aus Entry
    return (*pte & ~0xFFF) | (virt_addr & 0xFFF);
}
