/**
 * Copyright (c) 2026 KibaOfficial
 * MIT License
 *
 * VFS - Virtual File System Implementation
 */

#include "vfs.h"
#include "vga.h"
#include "string.h"

// Gobale Node Tabelle
typedef struct {
  char    path[VFS_NAME_MAX];
  vnode_t *node;
} vfs_entry_t;

static vfs_entry_t vfs_table[VFS_MAX_NODES];
static int vfs_count = 0;

void vfs_init(void) {
  for (int i = 0; i < VFS_MAX_NODES; i++) {
    vfs_table[i].node = NULL;
    vfs_table[i].path[0] = '\0';
  }
  vfs_count = 0;
}

int vfs_register(const char *path, vnode_t *node) {
    if (vfs_count >= VFS_MAX_NODES) {
        vga_println("[VFS] ERROR: Node table full!");
        return -1;
    }
    strncpy(vfs_table[vfs_count].path, path, VFS_NAME_MAX);
    vfs_table[vfs_count].node = node;
    vfs_count++;
    return 0;
}

vnode_t* vfs_find(const char *path) {
    for (int i = 0; i < vfs_count; i++) {
        if (strncmp(vfs_table[i].path, path, VFS_NAME_MAX) == 0) {
            return vfs_table[i].node;
        }
    }
    return NULL;
}

int vfs_open(const char *path) {
    vnode_t *node = vfs_find(path);
    if (!node || !node->ops || !node->ops->open) return -1;
    return node->ops->open(node);
}

int vfs_read(const char *path, void *buf, uint64_t len) {
    vnode_t *node = vfs_find(path);
    if (!node || !node->ops || !node->ops->read) return -1;
    return node->ops->read(node, buf, len);
}

int vfs_write(const char *path, const void *buf, uint64_t len) {
    vnode_t *node = vfs_find(path);
    if (!node || !node->ops || !node->ops->write) return -1;
    return node->ops->write(node, buf, len);
}

int vfs_close(const char *path) {
    vnode_t *node = vfs_find(path);
    if (!node || !node->ops || !node->ops->close) return -1;
    return node->ops->close(node);
}

// Mount-Tabelle (separat von der Node-Tabelle)
static vfs_mount_t vfs_mounts[VFS_MAX_NODES];
static int vfs_mount_count = 0;

int vfs_mount(const char *path, vnode_t *node) {
    if (vfs_mount_count >= VFS_MAX_NODES) {
        vga_println("[VFS] ERROR: Mount table full!");
        return -1;
    }
    strncpy(vfs_mounts[vfs_mount_count].path, path, VFS_NAME_MAX);
    vfs_mounts[vfs_mount_count].node = node;
    vfs_mount_count++;
    vga_print("[VFS] mounted: ");
    vga_println(path);
    return 0;
}

void vfs_list_mounts(void (*callback)(const char *path)) {
    for (int i = 0; i < vfs_mount_count; i++) {
        callback(vfs_mounts[i].path);
    }
}

int vfs_readdir(const char *path, void (*callback)(const char *name)) {
    // erst in mount tabelle suchen
    for (int i = 0; i < vfs_mount_count; i++) {
        if (strncmp(vfs_mounts[i].path, path, VFS_NAME_MAX) == 0) {
            vnode_t *node = vfs_mounts[i].node;
            if (node && node->ops && node->ops->readdir) {
                return node->ops->readdir(node, callback);
            }
            return -1;
        }
    }
    // dann in node tabelle suchen
    vnode_t *node = vfs_find(path);
    if (node && node->ops && node->ops->readdir) {
        return node->ops->readdir(node, callback);
    }
    return -1;
}

void vfs_list_submounts(const char *parent, void (*callback)(const char *path)) {
    for (int i = 0; i < vfs_mount_count; i++) {
        const char *mp = vfs_mounts[i].path;
        if (strncmp(mp, parent, VFS_NAME_MAX) == 0) continue;

        // check ob mp direkt unter parent liegt
        int plen = 0;
        while (parent[plen]) plen++;

        if (parent[plen-1] == '/') {
            if (mp[0] == '/' && mp[1] != '\0') {
                int has_sub = 0;
                for (int j = 1; mp[j]; j++) {
                    if (mp[j] == '/') { has_sub = 1; break; }
                }
                if (!has_sub) callback(mp);
            }
        }
    }
}