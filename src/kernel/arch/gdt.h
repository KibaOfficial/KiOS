// Copyright (c) 2026 KibaOfficial
// GDT-Initialisierung für x86_64 mit TSS
//
#ifndef KIOS_GDT_H
#define KIOS_GDT_H

#include "types.h"

// Segment Selectors
// WICHTIG: Reihenfolge ist für SYSRET optimiert!
// SYSRET im 64-bit Mode lädt: SS = STAR[63:48]+8, CS = STAR[63:48]+16
// Mit STAR[63:48] = 0x10 (Kernel Data):
//   SS = 0x10 + 8 = 0x18 (User Data)
//   CS = 0x10 + 16 = 0x20 (User Code)
// Also muss User Data (0x18) VOR User Code (0x20) kommen!
#define GDT_KERNEL_CODE 0x08
#define GDT_KERNEL_DATA 0x10
#define GDT_USER_DATA   0x18    // SYSRET: STAR[63:48] + 8 = 0x18
#define GDT_USER_CODE   0x20    // SYSRET: STAR[63:48] + 16 = 0x20
#define GDT_TSS         0x28

// Segment Selectors mit RPL (Request Privilege Level) für Ring 3
#define GDT_USER_DATA_RPL3  (GDT_USER_DATA | 3)  // 0x1B
#define GDT_USER_CODE_RPL3  (GDT_USER_CODE | 3)  // 0x23

void gdt_init(void* tss_ptr, uint16_t tss_size);

#endif // KIOS_GDT_H
