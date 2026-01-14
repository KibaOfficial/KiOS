#include "../commands.h"
#include "../vga.h"

void cmd_clear(const char* args) {
    (void)args;
    vga_clear();
}
