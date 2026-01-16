// Copyright (c) 2026 KibaOfficial
// GDT-Initialisierung für x86_64 mit TSS
//
#include "gdt.h"
#include <string.h>
// #include <stdint.h>

// GDT-Einträge (Null, Code, Data, TSS Low, TSS High)
__attribute__((aligned(16)))
uint64_t gdt[6];

// GDT-Pointer-Struktur
struct __attribute__((packed)) {
    uint16_t limit;
    uint64_t base;
} gdt_ptr;

void gdt_init(void* tss_ptr, uint16_t tss_size) {
    memset(gdt, 0, sizeof(gdt));
    // Null-Deskriptor
    gdt[0] = 0;
    // Kernel Code Segment (0x08)
    gdt[1] = 0x00af9a000000ffff;
    // Kernel Data Segment (0x10)
    gdt[2] = 0x00af92000000ffff;
    // TSS-Deskriptor (0x18/0x20)
    uint64_t base = (uint64_t)tss_ptr;
    uint64_t limit = tss_size - 1;
    gdt[3] = (limit & 0xFFFF) | ((base & 0xFFFFFF) << 16) | (0x89ULL << 40) | ((limit & 0xF0000ULL) << 32) | ((base & 0xFF000000ULL) << 32);
    gdt[4] = (base >> 32);
    // GDT-Pointer setzen
    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base = (uint64_t)&gdt;
    // Lade GDT
    asm volatile ("lgdt %0" : : "m"(gdt_ptr));

    // CS neu laden mit far return (0x08 = Code Segment)
    asm volatile (
        "pushq $0x08\n"          // Code Segment Selector
        "leaq 1f(%%rip), %%rax\n"  // Adresse des nächsten Labels
        "pushq %%rax\n"
        "lretq\n"                // Far return
        "1:\n"
        : : : "rax"
    );

    // Data Segment Register neu laden (0x10 = Data Segment)
    asm volatile (
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        : : : "ax"
    );

    // Lade TSS (0x18 = TSS Segment)
    asm volatile ("ltr %%ax" : : "a"(0x18));
}
