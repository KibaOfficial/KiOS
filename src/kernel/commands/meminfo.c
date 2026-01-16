#include "../commands.h"
#include "../vga.h"
#include "../mm/pmm.h"
#include "../mm/vmm.h"
#include "../mm/heap.h"

void cmd_meminfo(const char* args) {
    (void)args;

    vga_println("");
    vga_println("=== Memory Information ===");
    vga_println("");

    // PMM Statistics
    vga_print_colored("Physical Memory Manager (PMM):", VGA_LIGHT_CYAN, VGA_BLACK);
    vga_println("");

    uint64_t total_pages = pmm_total_pages();
    uint64_t used_pages = pmm_used_pages();
    uint64_t free_pages = total_pages - used_pages;

    vga_print("  Total Pages:  ");
    vga_print_dec(total_pages);
    vga_print(" (");
    vga_print_dec(total_pages * 4);
    vga_println(" KB)");

    vga_print("  Used Pages:   ");
    vga_print_dec(used_pages);
    vga_print(" (");
    vga_print_dec(used_pages * 4);
    vga_println(" KB)");

    vga_print("  Free Pages:   ");
    vga_print_dec(free_pages);
    vga_print(" (");
    vga_print_dec(free_pages * 4);
    vga_println(" KB)");

    // Calculate percentage
    uint64_t usage_percent = (used_pages * 100) / total_pages;
    vga_print("  Usage:        ");
    vga_print_dec(usage_percent);
    vga_println("%");

    vga_println("");

    // VMM Statistics
    vga_print_colored("Virtual Memory Manager (VMM):", VGA_LIGHT_CYAN, VGA_BLACK);
    vga_println("");

    vga_print("  PML4 Address: ");
    vga_print_hex(vmm_get_cr3());
    vga_println("");

    vga_print("  Page Size:    ");
    vga_println("4 KB");

    vga_print("  Levels:       ");
    vga_println("4 (PML4 -> PDPT -> PD -> PT)");

    vga_println("");

    // Heap Statistics
    vga_print_colored("Kernel Heap:", VGA_LIGHT_CYAN, VGA_BLACK);
    vga_println("");

    uint64_t heap_alloc = heap_total_allocated();
    uint64_t heap_size = heap_current_size();

    vga_print("  Base Address: ");
    vga_print_hex(0xFFFF800000000000ULL);
    vga_println("");

    vga_print("  Allocated:    ");
    vga_print_dec(heap_alloc);
    vga_println(" bytes");

    vga_print("  Current Size: ");
    vga_print_dec(heap_size);
    vga_println(" bytes");

    // Heap pages mapped
    uint64_t heap_pages = (heap_size + 4095) / 4096;
    vga_print("  Pages Mapped: ");
    vga_print_dec(heap_pages);
    vga_println("");

    vga_println("");
    vga_println("=========================");
    vga_println("");
}
