#include "../commands.h"
#include "../vga.h"
#include "../io.h"
#include "../types.h"

void cmd_reboot(const char* args) {
    (void)args;
    vga_println("Rebooting...");
    uint8_t good = 0x02;
    while (good & 0x02) {
        good = inb(0x64);
    }
    outb(0x64, 0xFE);
    __asm__ volatile ("lidt 0");
    __asm__ volatile ("int $0");
    for (;;) {
        __asm__ volatile ("hlt");
    }
}
