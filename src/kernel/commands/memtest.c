#include "../commands.h"
#include "../vga.h"
#include "../mm/pmm.h"
#include "../mm/vmm.h"
#include "../mm/heap.h"

#define TEST_PAGES 50
#define TEST_HEAP_ALLOCS 100

void cmd_memtest(const char* args) {
    (void)args;

    vga_println("");
    vga_println("=== Memory Stress Test ===");
    vga_println("");

    // Test 1: PMM Allocation
    vga_print_colored("Test 1: PMM Page Allocation", VGA_YELLOW, VGA_BLACK);
    vga_println("");
    vga_print("  Allocating ");
    vga_print_dec(TEST_PAGES);
    vga_println(" pages...");

    void* pages[TEST_PAGES];
    for (int i = 0; i < TEST_PAGES; i++) {
        pages[i] = pmm_alloc_page();
        if (!pages[i]) {
            vga_print_colored("  [FAIL] Failed to allocate page ", VGA_LIGHT_RED, VGA_BLACK);
            vga_print_dec(i);
            vga_println("");
            return;
        }
    }
    vga_print_colored("  [PASS] All pages allocated!", VGA_LIGHT_GREEN, VGA_BLACK);
    vga_println("");

    // Test 2: VMM Mapping
    vga_print_colored("Test 2: VMM Page Mapping", VGA_YELLOW, VGA_BLACK);
    vga_println("");
    vga_print("  Mapping ");
    vga_print_dec(TEST_PAGES);
    vga_println(" pages...");

    uint64_t virt_base = 0xFFFF900000000000ULL;
    for (int i = 0; i < TEST_PAGES; i++) {
        uint64_t virt_addr = virt_base + (i * 0x1000);
        vmm_map_page(virt_addr, (uint64_t)pages[i], PAGE_PRESENT | PAGE_WRITE);

        // Verify mapping
        uint64_t resolved = vmm_virt_to_phys(virt_addr);
        if (resolved != (uint64_t)pages[i]) {
            vga_print_colored("  [FAIL] Mapping mismatch at page ", VGA_LIGHT_RED, VGA_BLACK);
            vga_print_dec(i);
            vga_println("");
            return;
        }
    }
    vga_print_colored("  [PASS] All pages mapped correctly!", VGA_LIGHT_GREEN, VGA_BLACK);
    vga_println("");

    // Test 3: Read/Write Test
    vga_print_colored("Test 3: Memory Read/Write", VGA_YELLOW, VGA_BLACK);
    vga_println("");
    vga_print("  Writing test pattern to ");
    vga_print_dec(TEST_PAGES);
    vga_println(" pages...");

    for (int i = 0; i < TEST_PAGES; i++) {
        uint64_t virt_addr = virt_base + (i * 0x1000);
        uint64_t* ptr = (uint64_t*)virt_addr;

        // Write unique pattern
        *ptr = 0xDEADBEEF00000000ULL | i;
    }

    // Read back and verify
    vga_println("  Reading back and verifying...");
    for (int i = 0; i < TEST_PAGES; i++) {
        uint64_t virt_addr = virt_base + (i * 0x1000);
        uint64_t* ptr = (uint64_t*)virt_addr;
        uint64_t expected = 0xDEADBEEF00000000ULL | i;

        if (*ptr != expected) {
            vga_print_colored("  [FAIL] Data mismatch at page ", VGA_LIGHT_RED, VGA_BLACK);
            vga_print_dec(i);
            vga_println("");
            return;
        }
    }
    vga_print_colored("  [PASS] All data verified!", VGA_LIGHT_GREEN, VGA_BLACK);
    vga_println("");

    // Test 4: Unmapping
    vga_print_colored("Test 4: VMM Page Unmapping", VGA_YELLOW, VGA_BLACK);
    vga_println("");
    vga_print("  Unmapping ");
    vga_print_dec(TEST_PAGES);
    vga_println(" pages...");

    for (int i = 0; i < TEST_PAGES; i++) {
        uint64_t virt_addr = virt_base + (i * 0x1000);
        vmm_unmap_page(virt_addr);

        // Verify unmapped
        uint64_t resolved = vmm_virt_to_phys(virt_addr);
        if (resolved != 0) {
            vga_print_colored("  [FAIL] Page still mapped at ", VGA_LIGHT_RED, VGA_BLACK);
            vga_print_dec(i);
            vga_println("");
            return;
        }
    }
    vga_print_colored("  [PASS] All pages unmapped!", VGA_LIGHT_GREEN, VGA_BLACK);
    vga_println("");

    // Test 5: Free pages
    vga_print_colored("Test 5: PMM Page Freeing", VGA_YELLOW, VGA_BLACK);
    vga_println("");
    vga_print("  Freeing ");
    vga_print_dec(TEST_PAGES);
    vga_println(" pages...");

    for (int i = 0; i < TEST_PAGES; i++) {
        pmm_free_page(pages[i]);
    }
    vga_print_colored("  [PASS] All pages freed!", VGA_LIGHT_GREEN, VGA_BLACK);
    vga_println("");

    // Test 6: Heap Allocations
    vga_print_colored("Test 6: Heap Allocations", VGA_YELLOW, VGA_BLACK);
    vga_println("");
    vga_print("  Allocating ");
    vga_print_dec(TEST_HEAP_ALLOCS);
    vga_println(" heap blocks...");

    void* heap_ptrs[TEST_HEAP_ALLOCS];
    for (int i = 0; i < TEST_HEAP_ALLOCS; i++) {
        heap_ptrs[i] = kmalloc(256);  // 256 bytes each
        if (!heap_ptrs[i]) {
            vga_print_colored("  [FAIL] kmalloc failed at ", VGA_LIGHT_RED, VGA_BLACK);
            vga_print_dec(i);
            vga_println("");
            return;
        }

        // Write test pattern
        uint8_t* ptr = (uint8_t*)heap_ptrs[i];
        for (int j = 0; j < 256; j++) {
            ptr[j] = (uint8_t)(i + j);
        }
    }

    // Verify heap data
    vga_println("  Verifying heap data...");
    for (int i = 0; i < TEST_HEAP_ALLOCS; i++) {
        uint8_t* ptr = (uint8_t*)heap_ptrs[i];
        for (int j = 0; j < 256; j++) {
            if (ptr[j] != (uint8_t)(i + j)) {
                vga_print_colored("  [FAIL] Heap data corruption at block ", VGA_LIGHT_RED, VGA_BLACK);
                vga_print_dec(i);
                vga_println("");
                return;
            }
        }
    }
    vga_print_colored("  [PASS] Heap test successful!", VGA_LIGHT_GREEN, VGA_BLACK);
    vga_println("");

    // Summary
    vga_println("");
    vga_print_colored("=== All Tests Passed! ===", VGA_LIGHT_GREEN, VGA_BLACK);
    vga_println("");
    vga_println("");
}
