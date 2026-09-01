/**
* Copyright (c) 2026 KibaOfficial
 * MIT License
 */
#include "commands.h"
#include "vga.h"
#include "ramdisk.h"
#include "vfs.h"

static void ls_print_entry(const char *name) {
 vga_println(name);
}

void cmd_ls(const char *args) {
 const char *path = (args && args[0] != '\0') ? args : "/";

 // Submounts anzeigen
 vfs_list_submounts(path, ls_print_entry);

 // Files im Backend anzeigen
 vfs_readdir(path, ls_print_entry);
}