/**
* Copyright (c) 2026 KibaOfficial
 * MIT License
 */
#include "commands.h"
#include "vga.h"
#include "vfs.h"

void cmd_ls(const char *args) {
    const char *path = (args && args[0] != '\0') ? args : "/";

    // Submounts unter diesem Pfad anzeigen (z.B. /dev unter /)
    vfs_list_submounts(path, (void(*)(const char *)) vga_println);

    // Einträge via readdir iterieren
    uint32_t index = 0;
    vfs_dirent_t *entry;
    while ((entry = vfs_readdir(path, index++)) != NULL) {
        if (entry->type == VFS_TYPE_DIR) vga_print("[d] ");
        else if (entry->type == VFS_TYPE_DEVICE) vga_print("[c] ");
        else vga_print("[f] ");
        vga_println(entry->name);
    }
}
