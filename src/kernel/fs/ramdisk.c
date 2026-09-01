/**
 * Copyright (c) 2026 KibaOfficial
 * MIT License
 *
 * Ramdisk - In-Memory Filesystem Implementation
 */

#include "ramdisk.h"
#include "vfs.h"
#include "../drivers/vga.h"
#include "string.h"
#include "../mm/heap.h"

static ramdisk_entry_t ramdisk[RAMDISK_MAX_FILES];

// =============================================================================
// Hilfsfunktion: Entry per Name finden
// =============================================================================
static ramdisk_entry_t *ramdisk_find(const char *name) {
    for (int i = 0; i < RAMDISK_MAX_FILES; i++) {
        if (ramdisk[i].used && strcmp(name, ramdisk[i].name) == 0) {
            return &ramdisk[i];
        }
    }
    return NULL;
}

// =============================================================================
// Public Functions
// =============================================================================

void ramdisk_init(void) {
    for (int i = 0; i < RAMDISK_MAX_FILES; i++) {
        ramdisk[i].used = 0;
        ramdisk[i].data = NULL;
        ramdisk[i].size = 0;
        ramdisk[i].name[0] = '\0';
    }
    vga_println("[RAMDISK] initialized");
}

int ramdisk_create(const char *name, const void *data, const uint64_t size) {
    // schon vorhanden?
    if (ramdisk_find(name)) {
        vga_println("[RAMDISK] ERROR: file already exists");
        return -1;
    }

    // freien slot finden
    for (int i = 0; i < RAMDISK_MAX_FILES; i++) {
        if (!ramdisk[i].used) {
            strncpy(ramdisk[i].name, name, RAMDISK_NAME_MAX);
            ramdisk[i].size = size;
            ramdisk[i].used = 1;

            if (size > 0 && data) {
                ramdisk[i].data = (uint8_t *) kmalloc(size);
                if (!ramdisk[i].data) {
                    vga_println("[RAMDISK] ERROR: out of memory");
                    ramdisk[i].used = 0;
                    return -1;
                }

                // manuell kopieren kein mcopy
                for (uint64_t j = 0; j < size; j++) {
                    ramdisk[i].data[j] = ((const uint8_t *) data)[j];
                }
            } else {
                ramdisk[i].data = NULL;
            }
            return 0;
        }
    }

    vga_println("[RAMDISK] ERROR: no free slots");
    return -1;
}

int ramdisk_read(const char *name, void *buf, const uint64_t len) {
    ramdisk_entry_t *entry = ramdisk_find(name);
    if (!entry || !entry->data) return -1;

    uint64_t to_read = len < entry->size ? len : entry->size;
    for (uint64_t i = 0; i < to_read; i++) {
        ((uint8_t *) buf)[i] = entry->data[i];
    }
    return (int) to_read;
}

int ramdisk_write(const char *name, const void *data, const uint64_t size) {
    ramdisk_entry_t *entry = ramdisk_find(name);
    if (!entry) return -1;

    // alten speicher freigeben (kfree ist no-op aber wird später gebraucht)
    kfree(entry->data);

    entry->data = (uint8_t *) kmalloc(size);
    if (!entry->data) return -1;

    entry->size = size;
    for (uint64_t i = 0; i < size; i++) {
        entry->data[i] = ((const uint8_t *) data)[i];
    }
    return 0;
}

int ramdisk_delete(const char *name) {
    ramdisk_entry_t *entry = ramdisk_find(name);
    if (!entry) return -1;

    kfree(entry->data);
    entry->data = NULL;
    entry->size = 0;
    entry->used = 0;
    entry->name[0] = '\0';
    return 0;
}

void ramdisk_list(void (*callback)(const char *name)) {
    for (int i = 0; i < RAMDISK_MAX_FILES; i++) {
        if (ramdisk[i].used) {
            callback(ramdisk[i].name);
        }
    }
}

// =============================================================================
// VFS Integration
// =============================================================================

static int ramdisk_vfs_open(vnode_t *node) {
    (void)node;
    return 0;
}

static int ramdisk_vfs_read(vnode_t *node, void *buf, uint64_t len) {
    // node->data enthält den dateinamen
    return ramdisk_read((const char*)node->data, buf, len);
}

static int ramdisk_vfs_readdir(vnode_t *node, void (*callback)(const char *name)) {
    (void)node;
    ramdisk_list(callback);
    return 0;
}

static int ramdisk_vfs_write(vnode_t *node, const void *buf, uint64_t len) {
    return ramdisk_write((const char*)node->data, buf, len);
}

static int ramdisk_vfs_close(vnode_t *node) {
    (void)node;
    return 0;
}

static vfs_ops_t ramdisk_ops = {
    .open  = ramdisk_vfs_open,
    .read  = ramdisk_vfs_read,
    .write = ramdisk_vfs_write,
    .close = ramdisk_vfs_close,
    .readdir = ramdisk_vfs_readdir,
};

vnode_t ramdisk_root_node = {
    .name = "ramdisk",
    .type = VFS_TYPE_DIR,
    .ops  = &ramdisk_ops,
    .data = NULL,
};