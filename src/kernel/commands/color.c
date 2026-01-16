#include "../commands.h"
#include "../vga.h"

void cmd_color(const char* args) {
    (void)args;
    vga_println("");
    vga_print("  ");
    for (int i = 0; i < 16; i++) {
        vga_set_color(VGA_BLACK, (vga_color_t)i);
        vga_print("  ");
    }
    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_println("");
    vga_print("  ");
    for (int i = 0; i < 16; i++) {
        vga_set_color((vga_color_t)i, VGA_BLACK);
        vga_print("##");
    }
    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_println("");
    vga_println("");
}
