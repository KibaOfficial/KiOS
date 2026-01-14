#include "../commands.h"
#include "../vga.h"

void cmd_info(const char* args) {
    (void)args;
    vga_println("");
    vga_print_colored("  KiOS ", VGA_LIGHT_CYAN, VGA_BLACK);
    vga_println("v0.2.0");
    vga_println("");
    vga_print("  Architecture: ");
    vga_println("x86_64 (Long Mode)");
    vga_print("  Video:        ");
    vga_println("VGA Text Mode 80x25");
    vga_print("  Kernel at:    ");
    vga_print_hex(0x100000);
    vga_println(" (1MB)");
    vga_print("  VGA Buffer:   ");
    vga_print_hex(0xB8000);
    vga_println("");
    vga_println("");
}
