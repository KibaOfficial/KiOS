/**
 * Copyright (c) 2026 KibaOfficial
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include "../commands.h"
#include "../vga.h"

void cmd_shutdown(const char* args) {
    (void)args;
    vga_println("Shutting down...");
    // Shutdown via ACPI
    // Dies ist hardwareabhängig und funktioniert möglicherweise nicht auf allen Systemen
    __asm__ volatile (
        "mov $0x604, %dx\n"  // ACPI Power Management Control Register
        "mov $0x2000, %ax\n" // Befehl zum Herunterfahren
        "out %ax, %dx\n"
    );
    for (;;) {
        __asm__ volatile ("hlt");
    }
    
}