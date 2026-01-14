#include "../commands.h"
#include "../vga.h"

void cmd_mem(const char* args) {
    (void)args;
    vga_println("");
    vga_println("  Memory Layout (approximate):");
    vga_println("");
    vga_print("  0x00000000 - 0x000003FF  ");
    vga_println("IVT (Real Mode)");
    vga_print("  0x00000400 - 0x000004FF  ");
    vga_println("BIOS Data Area");
    vga_print("  0x00007C00 - 0x00007DFF  ");
    vga_println("Bootloader Stage 1");
    vga_print("  0x00007E00 - 0x0000BDFF  ");
    vga_println("Bootloader Stage 2");
    vga_print("  0x00010000 - 0x00017FFF  ");
    vga_println("Kernel Load Buffer");
    vga_print("  0x000A0000 - 0x000BFFFF  ");
    vga_println("VGA Memory");
    vga_print("  0x000B8000 - 0x000B8F9F  ");
    vga_println("VGA Text Buffer");
    vga_print("  0x00100000 - ...         ");
    vga_println("Kernel (1MB+)");
    vga_print("  0x00200000               ");
    vga_println("Stack Top");
    vga_println("");
}
