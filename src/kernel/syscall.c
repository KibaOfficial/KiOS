// Copyright (c) 2026 KibaOfficial
// Syscall Interface für x86_64 mit syscall/sysret
//
#include "syscall.h"
#include "gdt.h"
#include "vga.h"
#include "string.h"

// MSR Adressen
#define MSR_GS_BASE         0xC0000101
#define MSR_KERNEL_GS_BASE  0xC0000102

// MSR Hilfsfunktionen
static inline uint64_t rdmsr(uint32_t msr) {
    uint32_t lo, hi;
    asm volatile ("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
    return ((uint64_t)hi << 32) | lo;
}

static inline void wrmsr(uint32_t msr, uint64_t value) {
    uint32_t lo = value & 0xFFFFFFFF;
    uint32_t hi = value >> 32;
    asm volatile ("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}

// Per-CPU Daten Struktur (für swapgs)
// Diese Struktur wird über GS-Segment adressiert
typedef struct {
    uint64_t kernel_stack;      // Offset 0x00: Kernel Stack Pointer (RSP0)
    uint64_t user_stack;        // Offset 0x08: Gespeicherter User Stack
    uint64_t current_task;      // Offset 0x10: Pointer zum aktuellen Task (optional)
} __attribute__((packed)) cpu_data_t;

// Statische Per-CPU Daten (für Single-CPU System)
static cpu_data_t cpu_data __attribute__((aligned(16)));

// Debug: Adresse der cpu_data Struktur abrufen
uint64_t syscall_get_cpu_data_addr(void) {
    return (uint64_t)&cpu_data;
}

void syscall_init(void) {
    // cpu_data explizit initialisieren
    cpu_data.kernel_stack = 0;
    cpu_data.user_stack = 0;
    cpu_data.current_task = 0;
    // 1. EFER: System Call Enable Bit setzen
    uint64_t efer = rdmsr(MSR_EFER);
    efer |= EFER_SCE;
    wrmsr(MSR_EFER, efer);

    // 2. STAR: Segment Selectors konfigurieren
    // Bits 31:0  = Reserved (EIP für 32-bit SYSCALL, nicht genutzt)
    // Bits 47:32 = SYSCALL CS (Kernel) - SS wird automatisch CS+8
    // Bits 63:48 = SYSRET base - CS=base+16, SS=base+8 (mit RPL 3)
    //
    // SYSCALL: CS = 0x08 (Kernel Code), SS = 0x10 (Kernel Data)
    // SYSRET:  Mit base=0x10: SS = 0x18 (User Data), CS = 0x20 (User Code)
    //
    // GDT Layout:
    //   0x00 = Null
    //   0x08 = Kernel Code (Index 1)
    //   0x10 = Kernel Data (Index 2)
    //   0x18 = User Data   (Index 3) - SYSRET: base + 8
    //   0x20 = User Code   (Index 4) - SYSRET: base + 16
    uint64_t star = 0;
    star |= ((uint64_t)GDT_KERNEL_CODE) << 32;  // Bits 47:32 = 0x08
    star |= ((uint64_t)GDT_KERNEL_DATA) << 48;  // Bits 63:48 = 0x10
    wrmsr(MSR_STAR, star);

    // 3. LSTAR: Syscall Entry Point (64-bit RIP)
    wrmsr(MSR_LSTAR, (uint64_t)syscall_entry);

    // 4. SFMASK: RFLAGS Bits die bei syscall gelöscht werden
    // Wir löschen IF (keine Interrupts während syscall entry),
    // TF (kein Debugging) und DF (String Ops aufwärts)
    wrmsr(MSR_SFMASK, SFMASK_IF | SFMASK_TF | SFMASK_DF);

    // 5. GS Base für swapgs konfigurieren
    // WICHTIG: Beide MSRs auf cpu_data setzen!
    // - GS_BASE: Wird vom Kernel direkt genutzt
    // - KERNEL_GS_BASE: Wird nach swapgs in syscall_entry aktiv
    //
    // Ablauf:
    // 1. Kernel läuft mit GS_BASE = &cpu_data
    // 2. jump_to_usermode macht swapgs: GS_BASE ↔ KERNEL_GS_BASE
    //    -> GS_BASE = &cpu_data (war KERNEL_GS_BASE)
    //    -> KERNEL_GS_BASE = &cpu_data (war GS_BASE)
    // 3. User macht syscall, syscall_entry macht swapgs:
    //    -> GS_BASE = &cpu_data, KERNEL_GS_BASE = &cpu_data
    //
    // Wenn beide auf &cpu_data zeigen, ist es egal wie oft swapgs gemacht wird!
    wrmsr(MSR_GS_BASE, (uint64_t)&cpu_data);
    wrmsr(MSR_KERNEL_GS_BASE, (uint64_t)&cpu_data);

    vga_print("[SYSCALL] GS_BASE set to: ");
    vga_print_hex((uint64_t)&cpu_data);
    vga_println(" - syscall/sysret ready.");
}

// Setzt den Kernel-Stack in der Per-CPU Struktur
void syscall_set_kernel_stack(uint64_t stack_top) {
    cpu_data.kernel_stack = stack_top;
}

// Der eigentliche Syscall Handler (wird von Assembly aufgerufen)
// Argumente kommen via Register: rdi=syscall_num, rsi=arg1, rdx=arg2, rcx=arg3
uint64_t syscall_handler(uint64_t syscall_num, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
    switch (syscall_num) {
        case SYS_EXIT:
            vga_println("[SYSCALL] sys_exit() called - halting");
            while (1) { asm volatile ("hlt"); }
            return 0;

        case SYS_WRITE:
            // arg1 = fd, arg2 = buffer, arg3 = length
            if (arg1 == 1) {  // stdout
                const char* buf = (const char*)arg2;
                for (uint64_t i = 0; i < arg3; i++) {
                    vga_putchar(buf[i]);
                }
                return arg3;
            }
            return (uint64_t)-1;

        case SYS_READ:
            // TODO: Implementieren
            return (uint64_t)-1;

        case SYS_YIELD:
            // TODO: Task yielden
            return 0;

        default:
            vga_print("[SYSCALL] Unknown syscall: ");
            vga_print_dec(syscall_num);
            vga_println("");
            return (uint64_t)-1;
    }
}
