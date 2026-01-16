// Copyright (c) 2026 KibaOfficial
// GDT-Initialisierung für x86_64 mit TSS
//
#ifndef KIOS_GDT_H
#define KIOS_GDT_H

#include "types.h"

void gdt_init(void* tss_ptr, uint16_t tss_size);

#endif // KIOS_GDT_H
