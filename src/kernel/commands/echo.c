#include "../commands.h"
#include "../vga.h"

void cmd_echo(const char* args) {
    vga_println(args);
}
