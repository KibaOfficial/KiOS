#include "../commands.h"
#include "../vga.h"
#include <string.h>

void cmd_fault(const char* args) {
    if (args == 0 || *args == 0) {
        vga_println("Usage: fault <div0|ud|pf>");
        vga_println("  div0  - Division durch Null (Exception 0)");
        vga_println("  ud    - Ungültiger Opcode (Exception 6)");
        vga_println("  pf    - Page Fault (Exception 14, nur wenn Paging aktiv)");
        return;
    }
    if (strncmp(args, "div0", 4) == 0) {
        vga_println("Trigger: Division durch Null!");
        volatile int zero = 0;
        volatile int x = 1 / zero;
        (void)x;
    } else if (strncmp(args, "ud", 2) == 0) {
        vga_println("Trigger: Ungültiger Opcode!");
        __asm__ volatile ("ud2");
    } else if (strncmp(args, "pf", 2) == 0) {
        vga_println("Trigger: Page Fault (nur mit Paging!)");
        volatile int* bad = (int*)0xDEADBEEF;
        *bad = 42;
    } else {
        vga_println("Unbekannter Fault-Typ. Nutze: div0, ud, pf");
    }
}
