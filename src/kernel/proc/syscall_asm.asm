; Copyright (c) 2026 KibaOfficial
; syscall/sysret Entry Point für x86_64
; MIT swapgs - korrekte Implementierung

[bits 64]

section .text

extern syscall_handler

global syscall_entry
syscall_entry:
    ; Wir kommen aus Ring 3. Die MSRs sind so:
    ;   GS_BASE = 0 (User)
    ;   KERNEL_GS_BASE = &cpu_data (Kernel)
    ;
    ; swapgs tauscht diese Werte:
    ;   GS_BASE = &cpu_data (jetzt können wir [gs:X] nutzen!)
    ;   KERNEL_GS_BASE = 0
    swapgs

    ; User-RSP in cpu_data.user_stack speichern [gs:0x08]
    mov [gs:0x08], rsp

    ; Kernel-RSP aus cpu_data.kernel_stack laden [gs:0x00]
    mov rsp, [gs:0x00]

    ; Register sichern
    push rcx                ; User RIP (von syscall in RCX gelegt)
    push r11                ; User RFLAGS (von syscall in R11 gelegt)
    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15

    ; Syscall-Argumente umshiften für C (System V ABI)
    ; syscall: RAX=num, RDI=arg1, RSI=arg2, RDX=arg3, R10=arg4
    ; C-ABI:   RDI=arg1, RSI=arg2, RDX=arg3, RCX=arg4
    mov rcx, rdx            ; arg3 -> rcx (4. C Argument)
    mov rdx, rsi            ; arg2 -> rdx (3. C Argument)
    mov rsi, rdi            ; arg1 -> rsi (2. C Argument)
    mov rdi, rax            ; syscall_num -> rdi (1. C Argument)

    sti                     ; Interrupts erlauben während Handler läuft
    call syscall_handler
    cli                     ; Interrupts aus für sysret

    ; Register wiederherstellen
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    pop rbx
    pop r11                 ; User RFLAGS
    pop rcx                 ; User RIP

    ; User-RSP wiederherstellen
    mov rsp, [gs:0x08]

    ; swapgs zurück: GS_BASE und KERNEL_GS_BASE wieder tauschen
    ;   GS_BASE = 0 (User)
    ;   KERNEL_GS_BASE = &cpu_data (Kernel)
    swapgs

    ; Zurück in Ring 3
    ; o64 prefix erzwingt 64-bit sysret
    o64 sysret


; =============================================================================
; jump_to_usermode(uint64_t user_stack, uint64_t user_rip)
; =============================================================================
; Springt von Ring 0 nach Ring 3 via IRETQ
;
; Die MSRs sind beim Aufruf:
;   GS_BASE = 0 (schon für User vorbereitet!)
;   KERNEL_GS_BASE = &cpu_data (für späteren syscall)
;
; Daher brauchen wir hier KEIN swapgs!

global jump_to_usermode
jump_to_usermode:
    cli

    ; User Data Segment in DS/ES laden
    mov ax, 0x1B            ; User Data Selector (0x18 | RPL 3)
    mov ds, ax
    mov es, ax

    ; IRETQ Stack Frame aufbauen (von unten nach oben):
    ;   SS      - User Stack Segment
    ;   RSP     - User Stack Pointer
    ;   RFLAGS  - User Flags (mit IF=1)
    ;   CS      - User Code Segment
    ;   RIP     - User Instruction Pointer

    push 0x1B               ; SS (User Data, RPL 3)
    push rdi                ; RSP (user_stack aus Parameter)
    push 0x202              ; RFLAGS (IF=1, Reserved Bit 1 = 1)
    push 0x23               ; CS (User Code 0x20 | RPL 3)
    push rsi                ; RIP (user_rip aus Parameter)

    ; KEIN swapgs nötig - MSRs sind schon korrekt für User!
    iretq
