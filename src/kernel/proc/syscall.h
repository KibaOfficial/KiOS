// Copyright (c) 2026 KibaOfficial
// Syscall Interface für x86_64 mit syscall/sysret
//
#ifndef KIOS_SYSCALL_H
#define KIOS_SYSCALL_H

#include "types.h"

// MSR Adressen für syscall/sysret
#define MSR_EFER        0xC0000080  // Extended Feature Enable Register
#define MSR_STAR        0xC0000081  // Segment Selectors für syscall/sysret
#define MSR_LSTAR       0xC0000082  // Long Mode SYSCALL Target (RIP)
#define MSR_CSTAR       0xC0000083  // Compatibility Mode SYSCALL Target (nicht genutzt)
#define MSR_SFMASK      0xC0000084  // SYSCALL Flag Mask (RFLAGS bits to clear)

// EFER Bits
#define EFER_SCE        (1 << 0)    // System Call Enable

// RFLAGS Bits die bei syscall gelöscht werden (SFMASK)
#define SFMASK_IF       (1 << 9)    // Interrupt Flag - Interrupts aus beim Syscall-Entry
#define SFMASK_TF       (1 << 8)    // Trap Flag - Kein Single-Stepping
#define SFMASK_DF       (1 << 10)   // Direction Flag - String Ops aufwärts

// Syscall Nummern
#define SYS_EXIT        0
#define SYS_WRITE       1
#define SYS_READ        2
#define SYS_YIELD       3

// Syscall Init
void syscall_init(void);

// Setzt den Kernel-Stack für syscall (in Per-CPU Daten)
void syscall_set_kernel_stack(uint64_t stack_top);

// Syscall Handler (wird von Assembly aufgerufen)
uint64_t syscall_handler(uint64_t syscall_num, uint64_t arg1, uint64_t arg2, uint64_t arg3);

// Assembly Entry Point (definiert in syscall_asm.asm)
extern void syscall_entry(void);

// Springt in Ring 3 und führt die angegebene Funktion aus
// user_stack = Top des User-Stacks (wächst nach unten)
// user_func = Adresse der User-Funktion
extern void jump_to_usermode(uint64_t user_stack, uint64_t user_func);

#endif // KIOS_SYSCALL_H
