/**
 * Copyright (c) 2026 KibaOfficial
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
#include "../shell.h"
#include "../vga.h"
#include "../pit.h"

void cmd_uptime(const char* args) {
    (void)args;

    uint64_t total_seconds = pit_get_uptime_seconds();
    uint64_t hours = total_seconds / 3600;
    uint64_t minutes = (total_seconds % 3600) / 60;
    uint64_t seconds = total_seconds % 60;

    vga_print("System uptime: ");
    vga_print_dec(hours);
    vga_print("h ");
    vga_print_dec(minutes);
    vga_print("m ");
    vga_print_dec(seconds);
    vga_println("s");
}
