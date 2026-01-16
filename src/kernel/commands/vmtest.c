#include "../commands.h"
#include "../vga.h"
#include "../mm/vmm.h"
#include "../mm/pmm.h"

void cmd_vmtest(const char* args) {
    (void)args;

    vga_println("");
    vga_println("=== VMM Test ===");
    vga_println("");

    // Test 1: Alloziere eine physische Page
    void* phys_page = pmm_alloc_page();
    if (!phys_page) {
        vga_print_colored("  [FAIL] Failed to allocate physical page!", VGA_LIGHT_RED, VGA_BLACK);
        vga_println("");
        return;
    }

    vga_print("  Allocated physical page at: ");
    vga_print_hex((uint64_t)phys_page);
    vga_println("");

    // Test 2: Mappe zu einer virtuellen Adresse
    uint64_t virt_addr = 0xFFFF800000001000ULL; // Kernel Heap Region
    vga_print("  Mapping to virtual address: ");
    vga_print_hex(virt_addr);
    vga_println("");

    vmm_map_page(virt_addr, (uint64_t)phys_page, PAGE_PRESENT | PAGE_WRITE);
    vga_println("  Mapping successful!");

    // Test 3: Checke ob Mapping funktioniert
    uint64_t resolved_phys = vmm_virt_to_phys(virt_addr);
    vga_print("  Resolved physical address: ");
    vga_print_hex(resolved_phys);
    vga_println("");

    if (resolved_phys == (uint64_t)phys_page) {
        vga_print_colored("  [PASS] Virtual to physical mapping correct!", VGA_LIGHT_GREEN, VGA_BLACK);
        vga_println("");
    } else {
        vga_print_colored("  [FAIL] Mapping mismatch!", VGA_LIGHT_RED, VGA_BLACK);
        vga_println("");
    }

    // Test 4: Schreibe und lese Daten über virtuelle Adresse
    vga_println("");
    vga_println("  Writing test data via virtual address...");
    uint64_t* ptr = (uint64_t*)virt_addr;
    *ptr = 0xDEADBEEFCAFEBABEULL;

    uint64_t read_value = *ptr;
    vga_print("  Read value: ");
    vga_print_hex(read_value);
    vga_println("");

    if (read_value == 0xDEADBEEFCAFEBABEULL) {
        vga_print_colored("  [PASS] Read/Write works correctly!", VGA_LIGHT_GREEN, VGA_BLACK);
        vga_println("");
    } else {
        vga_print_colored("  [FAIL] Read/Write failed!", VGA_LIGHT_RED, VGA_BLACK);
        vga_println("");
    }

    // Test 5: Unmap
    vga_println("");
    vga_println("  Unmapping page...");
    vmm_unmap_page(virt_addr);

    uint64_t resolved_after_unmap = vmm_virt_to_phys(virt_addr);
    if (resolved_after_unmap == 0) {
        vga_print_colored("  [PASS] Unmapping successful!", VGA_LIGHT_GREEN, VGA_BLACK);
        vga_println("");
    } else {
        vga_print_colored("  [FAIL] Page still mapped!", VGA_LIGHT_RED, VGA_BLACK);
        vga_println("");
    }

    // Test 6: Free physical page
    pmm_free_page(phys_page);
    vga_println("  Freed physical page");

    vga_println("");
    vga_println("=== VMM Test Complete ===");
    vga_println("");
}
