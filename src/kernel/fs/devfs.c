/**
 * Copyright (c) 2026 KibaOfficial
 * MIT License
 *
 * DevFS - /dev/ Filesystem Implementation
 */
#include "devfs.h"
#include "vfs.h"
#include "vga.h"
#include "keyboard.h"
#include "string.h"

// =============================================================================
// /dev/stdout — schreibt auf VGA
// =============================================================================

static int devstdout_open(vnode_t *node) {
    (void)node;
    return 0;
}

static int devstdout_read(vnode_t *node, void *buf, uint64_t len) {
    (void)node; (void)buf; (void)len;
    return -1;  // stdout ist nicht lesbar
}

static int devstdout_write(vnode_t *node, const void *buf, uint64_t len) {
    (void)node;
    const char *str = (const char *)buf;
    for (uint64_t i = 0; i < len; i++) {
        vga_putchar(str[i]);
    }
    return (int)len;
}

static int devstdout_close(vnode_t *node) {
    (void)node;
    return 0;
}

static vfs_ops_t devstdout_ops = {
    .open  = devstdout_open,
    .read  = devstdout_read,
    .write = devstdout_write,
    .close = devstdout_close,
};

static vnode_t devstdout_node = {
    .name = "stdout",
    .type = VFS_TYPE_DEVICE,
    .ops  = &devstdout_ops,
    .data = NULL,
};

// =============================================================================
// /dev/stdin — liest vom Keyboard-Buffer
// =============================================================================

static int devstdin_open(vnode_t *node) {
    (void)node;
    return 0;
}

static int devstdin_read(vnode_t *node, void *buf, uint64_t len) {
    (void)node;
    char *dst = (char *)buf;
    for (uint64_t i = 0; i < len; i++) {
        dst[i] = kb_getchar();  // blockiert bis Taste gedrückt
    }
    return (int)len;
}

static int devstdin_write(vnode_t *node, const void *buf, uint64_t len) {
    (void)node; (void)buf; (void)len;
    return -1;  // stdin ist nicht beschreibbar
}

static int devstdin_close(vnode_t *node) {
    (void)node;
    return 0;
}

static vfs_ops_t devstdin_ops = {
    .open  = devstdin_open,
    .read  = devstdin_read,
    .write = devstdin_write,
    .close = devstdin_close,
};

static vnode_t devstdin_node = {
    .name = "stdin",
    .type = VFS_TYPE_DEVICE,
    .ops  = &devstdin_ops,
    .data = NULL,
};

// =============================================================================
// devfs_init — registriert alle /dev/ Nodes im VFS
// =============================================================================

void devfs_init(void) {
    vfs_register("/dev/stdout", &devstdout_node);
    vfs_register("/dev/stdin",  &devstdin_node);
    vfs_mount("/dev", &devfs_root_node);
    vga_println("[VFS] /dev/stdout registered");
    vga_println("[VFS] /dev/stdin  registered");
}

// =============================================================================
// /dev/ root node — für ls /dev
// =============================================================================

static int devfs_readdir(vnode_t *node, void (*callback)(const char *name)) {
    (void)node;
    callback("stdout");
    callback("stdin");
    return 0;
}

static vfs_ops_t devfs_root_ops = {
    .open    = NULL,
    .read    = NULL,
    .write   = NULL,
    .close   = NULL,
    .readdir = devfs_readdir,
};

vnode_t devfs_root_node = {
    .name = "dev",
    .type = VFS_TYPE_DIR,
    .ops  = &devfs_root_ops,
    .data = NULL,
};