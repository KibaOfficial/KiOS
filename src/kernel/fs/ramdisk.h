/**
 * Copyright (c) 2026 KibaOfficial
 * MIT License
 *
 * Ramdisk - In-Memory Filesystem
 * Einfaches Dateisystem komplett im RAM.
 * Dient als Root-Filesystem unter /.
 */
#ifndef KIOS_RAMDISK_H
#define KIOS_RAMDISK_H

#include "types.h"
#include "vfs.h"

#define RAMDISK_MAX_FILES 32
#define RAMDISK_NAME_MAX 64
#define RAMDISK_FILE_MAX (64 * 1024)

typedef struct {
  char     name[RAMDISK_NAME_MAX]; // Dateiname
  uint8_t *data;                   // Pointer auf inhalt
  uint64_t size;                   // aktuelle größe in bytes
  int      used;                   // 1 = belegt, 0 = frei
} ramdisk_entry_t;

// init ramdisk
void ramdisk_init(void);

// file anlegen: gibt 0 bei success und -1 bei error
int ramdisk_create(const char *name, const void *data, uint64_t size);

// file lesen: gibt gelesene bytes zurück und -1 bei error
int ramdisk_read(const char *name, void *buf, uint64_t len);

// file schreiben/override: 0 bei success, -1 bei error
int ramdisk_write(const char *name, const void *data, uint64_t size);

// file löschen: 0 bei success, -1 bei error
int ramdisk_delete(const char *name);

// alle files auflisten für ls u.a.
// ruft callback(name) für jeden entry
void ramdisk_list(void (*callback)(const char *name));

// vfs node für ramdisk
extern vnode_t ramdisk_root_node;

#endif /* KIOS_RAMDISK_H */