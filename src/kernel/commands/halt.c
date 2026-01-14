#include "../commands.h"
#include "../vga.h"

void cmd_halt(const char* args) {
    (void)args;
    vga_println("System halted. You can turn off your computer.");
    vga_println("");
    __asm__ volatile ("cli");
    for (;;) {
        __asm__ volatile ("hlt");
    }
}
