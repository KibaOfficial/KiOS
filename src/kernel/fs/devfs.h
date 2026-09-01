/**
 * Copyright (c) 2026 KibaOfficial
 * MIT License
 *
 * DevFS - /dev/ Filesystem
 * Registriert /dev/stdin und /dev/stdout als VFS-Nodes.
 */
#ifndef KIOS_DEVFS_H
#define KIOS_DEVFS_H

#include "vfs.h"

void devfs_init(void);

extern vnode_t devfs_root_node;

#endif /* KIOS_DEVFS_H */