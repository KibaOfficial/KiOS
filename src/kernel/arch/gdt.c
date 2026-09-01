#include "gdt.h"
#include <string.h>

__attribute__((aligned(16)))
static uint64_t gdt[16]; 

static struct __attribute__((packed)) {
    uint16_t limit;
    uint64_t base;
} gdt_ptr;

void gdt_init(void* tss_ptr, uint16_t tss_size) {
    memset(gdt, 0, sizeof(gdt));

    gdt[0] = 0; // Null
    gdt[1] = 0x00af9a000000ffff; // 0x08: Kernel Code
    gdt[2] = 0x00af92000000ffff; // 0x10: Kernel Data
    gdt[3] = 0x00aff2000000ffff; // 0x18: User Data (DPL 3) -> RPL 3 = 0x1B
    gdt[4] = 0x00affa000000ffff; // 0x20: User Code (DPL 3) -> RPL 3 = 0x23

    // TSS
    uint64_t tss_base = (uint64_t)tss_ptr;
    gdt[5] = (tss_size - 1) | ((tss_base & 0xFFFFFF) << 16) | (0x89ULL << 40) | ((tss_base & 0xFF000000) << 32);
    gdt[6] = (tss_base >> 32);

    gdt_ptr.limit = (8 * 7) - 1;
    gdt_ptr.base = (uint64_t)&gdt;

    asm volatile("lgdt %0" : : "m"(gdt_ptr));

    // Segmente neuladen
    asm volatile(
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%ss\n"
        "pushq $0x08\n"
        "leaq 1f(%%rip), %%rax\n"
        "pushq %%rax\n"
        "lretq\n"
        "1:\n"
        ::: "rax", "memory"
    );

    asm volatile("ltr %%ax" : : "a"(0x28));
}