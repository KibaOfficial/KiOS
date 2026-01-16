#include "../commands.h"
#include "../vga.h"
#include "../string.h"
#include "../shell.h"

extern const shell_command_t shell_commands[];
extern const int shell_commands_count;

void cmd_help(const char* args) {
    (void)args;
    vga_println("");
    vga_print_colored("  KiOS Shell Commands", VGA_YELLOW, VGA_BLACK);
    vga_println("");
    vga_println("  ================================");
    vga_println("");
    for (int i = 0; i < shell_commands_count; i++) {
        vga_print_colored("  ", VGA_WHITE, VGA_BLACK);
        vga_print_colored(shell_commands[i].name, VGA_LIGHT_CYAN, VGA_BLACK);
        int pad = 12 - (int)strlen(shell_commands[i].name);
        for (int j = 0; j < pad; j++) vga_putchar(' ');
        vga_println(shell_commands[i].help);
    }
    vga_println("");
}
