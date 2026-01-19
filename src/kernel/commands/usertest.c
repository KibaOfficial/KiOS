// Copyright (c) 2026 KibaOfficial
// usertest command - Test Ring 3 / User Mode
//
#include "../commands.h"
#include "../vga.h"
#include "../syscall.h"
#include "../tss.h"
#include "../mm/heap.h"
#include "../mm/vmm.h"
#include "../mm/pmm.h"
#include "../string.h"

// Einfacher User-Code als Bytecode (Position Independent)
// Dieser Code macht:
//   1. sys_write(1, msg, 14) - schreibt "Hello Ring 3!\n"
//   2. sys_exit(0) - beendet
//
// Bytecode Layout:
//   Offset 0x00: lea rsi, [rip+X]  ; 7 bytes - X zeigt auf String
//   Offset 0x07: mov rax, 1        ; 7 bytes
//   Offset 0x0E: mov rdi, 1        ; 7 bytes
//   Offset 0x15: mov rdx, 14       ; 7 bytes
//   Offset 0x1C: syscall           ; 2 bytes
//   Offset 0x1E: mov rax, 0        ; 7 bytes
//   Offset 0x25: syscall           ; 2 bytes
//   Offset 0x27: jmp $             ; 2 bytes
//   Offset 0x29: "Hello Ring 3!\n" ; 15 bytes (inkl. \0)
//
// lea rsi, [rip+X] am Offset 0x00 zeigt nach seiner Ausführung auf Offset 0x07 (RIP nach lea)
// Um auf 0x29 zu kommen: X = 0x29 - 0x07 = 0x22 = 34
static const uint8_t user_code[] = {
    // lea rsi, [rip+34]  ; buffer = message (RIP-relative)
    0x48, 0x8d, 0x35, 0x22, 0x00, 0x00, 0x00,   // Offset 0x00-0x06
    // mov rax, 1          ; SYS_WRITE = 1
    0x48, 0xc7, 0xc0, 0x01, 0x00, 0x00, 0x00,   // Offset 0x07-0x0D
    // mov rdi, 1          ; fd = stdout
    0x48, 0xc7, 0xc7, 0x01, 0x00, 0x00, 0x00,   // Offset 0x0E-0x14
    // mov rdx, 14         ; length (strlen("Hello Ring 3!\n"))
    0x48, 0xc7, 0xc2, 0x0e, 0x00, 0x00, 0x00,   // Offset 0x15-0x1B
    // syscall
    0x0f, 0x05,                                  // Offset 0x1C-0x1D

    // --- sys_exit(0) ---
    // mov rax, 0          ; SYS_EXIT = 0
    0x48, 0xc7, 0xc0, 0x00, 0x00, 0x00, 0x00,   // Offset 0x1E-0x24
    // syscall
    0x0f, 0x05,                                  // Offset 0x25-0x26

    // --- Infinite loop (safety) ---
    // jmp $
    0xeb, 0xfe,                                  // Offset 0x27-0x28

    // --- Message string (at offset 0x29) ---
    'H', 'e', 'l', 'l', 'o', ' ', 'R', 'i', 'n', 'g', ' ', '3', '!', '\n', '\0'
};

// User-Space Adressen (niedrig im virtuellen Adressraum)
#define USER_CODE_VADDR  0x400000   // 4MB - typische User-Code Adresse
#define USER_STACK_VADDR 0x800000   // 8MB - User Stack

void cmd_usertest(const char* args) {
    (void)args;

    // 1. Pages allozieren
    uint64_t code_phys = (uint64_t)pmm_alloc_page();
    uint64_t stack_phys = (uint64_t)pmm_alloc_page();

    if (!code_phys || !stack_phys) {
        vga_println("ERROR: Memory allocation failed!");
        return;
    }

    // 2. Mappen mit PAGE_USER!
    vmm_map_page(USER_CODE_VADDR, code_phys, PAGE_PRESENT | PAGE_WRITE | PAGE_USER);
    vmm_map_page(USER_STACK_VADDR, stack_phys, PAGE_PRESENT | PAGE_WRITE | PAGE_USER);

    // 3. Code kopieren
    memcpy((void*)USER_CODE_VADDR, user_code, sizeof(user_code));

    // 4. Stacks vorbereiten
    uint64_t user_stack_top = (USER_STACK_VADDR + PAGE_SIZE) - 16;

    // Kernel Stack für Syscalls/Interrupts
    static uint8_t secure_kernel_stack[8192] __attribute__((aligned(16)));
    uint64_t k_stack_top = (uint64_t)secure_kernel_stack + sizeof(secure_kernel_stack);

    // TSS und Syscall-Stack setzen
    tss_set_kernel_stack(k_stack_top);
    syscall_set_kernel_stack(k_stack_top);

    // 5. TLB FLUSH
    vmm_set_cr3(vmm_get_cr3());

    // 6. Sprung in User Mode
    jump_to_usermode(user_stack_top, USER_CODE_VADDR);
}
