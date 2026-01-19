; Copyright (c) 2026 KibaOfficial
; syscall/sysret Entry Point für x86_64
; SIMPLIFIED VERSION - ohne swapgs für Debugging

[bits 64]

section .data
; Statischer Kernel-Stack für syscall (8KB)
align 16
syscall_stack_bottom:
    times 8192 db 0
syscall_stack_top:

; Speicher für User-RSP
saved_user_rsp: dq 0

section .text

extern syscall_handler

global syscall_entry
syscall_entry:
    ; KEIN swapgs - wir nutzen einen statischen Stack

    ; User-RSP sichern
    mov [rel saved_user_rsp], rsp

    ; Kernel-Stack laden (statisch definiert)
    lea rsp, [rel syscall_stack_top]

    ; Register sichern
    push rcx                ; User RIP
    push r11                ; User RFLAGS
    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15

    ; Syscall-Argumente umshiften für C (System V ABI)
    mov rcx, rdx
    mov rdx, rsi
    mov rsi, rdi
    mov rdi, rax

    sti
    call syscall_handler
    cli

    ; Register wiederherstellen
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    pop rbx
    pop r11
    pop rcx

    ; User-RSP wiederherstellen
    mov rsp, [rel saved_user_rsp]

    o64 sysret

global jump_to_usermode
jump_to_usermode:
    cli

    ; User Data Segment laden
    mov ax, 0x1B
    mov ds, ax
    mov es, ax

    ; IRETQ Stack Frame aufbauen
    push 0x1B               ; SS (User Data, RPL 3)
    push rdi                ; RSP (user_stack)
    push 0x202              ; RFLAGS (IF gesetzt)
    push 0x23               ; CS (User Code, RPL 3)
    push rsi                ; RIP (user_rip)

    ; KEIN swapgs - direkt springen
    iretq