/**
* Copyright (c) 2026 KibaOfficial
 * MIT License
 */
#include "../commands.h"
#include "../drivers/vga.h"
#include "../fs/ramdisk.h"
#include "string.h"

void cmd_cat(const char *args) {
    if (!args || args[0] == '\0') {
        vga_println("Usage: cat <filename>");
        return;
    }

    // Erst checken ob es ein Directory/Mount ist
    vfs_dirent_t *entry = vfs_readdir(args, 0);
    if (entry != NULL) {
        vga_print("cat: ");
        vga_print(args);
        vga_println(": Is a directory");
        return;
    }

    // Normal lesen
    char buf[1024];
    int bytes = ramdisk_read(args, buf, sizeof(buf) - 1);
    if (bytes < 0) {
        vga_print("cat: ");
        vga_print(args);
        vga_println(": No such file");
        return;
    }

    buf[bytes] = '\0';
    vga_print(buf);
}