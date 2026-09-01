/**
 * Copyright (c) 2026 KibaOfficial
 * MIT License
 *
 * VFS - Virtual File System
 * Abstrakte Schnittstelle für alle Dateisystem-Operationen in KiOS.
 */

#ifndef KIOS_VFS_H
#define KIOS_VFS_H

#include "types.h"

// Max anzhal offener files
#define VFS_MAX_NODES   32
// max länge datei namen
#define VFS_NAME_MAX    64

// Forward declaration
typedef struct vnode vnode_t;

// operations die jedes vfs backend implementieren muss
typedef struct vfs_ops {
  int (*open)   (vnode_t *node);
  int (*read)   (vnode_t *node, void *buf, uint64_t len);
  int (*write)  (vnode_t *node, const void *buf, uint64_t len);
  int (*close)  (vnode_t *node);
  int (*readdir)(vnode_t *node, void (*callback)(const char *name));
} vfs_ops_t;

// node types
typedef enum {
  VFS_TYPE_FILE,  // Normale Datei
  VFS_TYPE_DIR,   // Verzeichnis
  VFS_TYPE_DEVICE,// Device (/dev/stdout etc.)
} vnode_type_t; 

// VFS node struktur
struct vnode {
  char          name[VFS_NAME_MAX];
  vnode_type_t  type;
  vfs_ops_t     *ops;
  void          *data;
};

// VFS initialisieren
void vfs_init(void);

// Node registrieren
int vfs_register(const char *path, vnode_t *node);

// Node per Pfad finden
vnode_t* vfs_find(const char *path);

// Operationen
int vfs_open (const char *path);
int vfs_read (const char *path, void *buf, uint64_t len);
int vfs_write(const char *path, const void *buf, uint64_t len);
int vfs_close(const char *path);

// Mount-Eintrag
typedef struct {
  char     path[VFS_NAME_MAX];
  vnode_t *node;
} vfs_mount_t;

// Mount registrieren (z.B. "/dev", "/proc")
int vfs_mount(const char *path, vnode_t *node);

// Alle Mounts auflisten (für ls)
void vfs_list_mounts(void (*callback)(const char *path));

// Verzeichnis auflisten via VFS
int vfs_readdir(const char *path, void (*callback)(const char *name));

void vfs_list_submounts(const char *parent, void (*callback)(const char *path));

#endif /* KIOS_VFS_H */