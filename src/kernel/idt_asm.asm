; Copyright (c) 2026 KibaOfficial
;
; This software is released under the MIT License.
; https://opensource.org/licenses/MIT

; KiOS - IDT Assembly Stubs
;
; Diese Datei enthält die ISR (Interrupt Service Routine) Stubs für alle
; Interrupts und Exceptions. Jeder Stub pusht die Interrupt-Nummer auf den
; Stack und springt zum gemeinsamen Handler.

[BITS 64]

; Externe C-Funktionen
extern isr_handler
extern irq_handler

; IDT laden
global idt_load
idt_load:
    lidt [rdi]      ; RDI enthält die Adresse des IDT-Pointers
    ret

; Makro für ISR ohne Error Code
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    push 0          ; Dummy Error Code (für einheitlichen Stack-Frame)
    push %1         ; Interrupt-Nummer
    jmp isr_common_stub
%endmacro

; Makro für ISR mit Error Code (CPU pusht automatisch)
%macro ISR_ERRCODE 1
global isr%1
isr%1:
    push %1         ; Interrupt-Nummer
    jmp isr_common_stub
%endmacro

; Makro für IRQ-Handler
%macro IRQ 2
global irq%1
irq%1:
    push 0          ; Dummy Error Code
    push %2         ; IRQ-Nummer (32 + IRQ)
    jmp irq_common_stub
%endmacro

; Exception-Handler (0-31)
ISR_NOERRCODE 0     ; Division By Zero
ISR_NOERRCODE 1     ; Debug
ISR_NOERRCODE 2     ; Non Maskable Interrupt
ISR_NOERRCODE 3     ; Breakpoint
ISR_NOERRCODE 4     ; Overflow
ISR_NOERRCODE 5     ; Bound Range Exceeded
ISR_NOERRCODE 6     ; Invalid Opcode
ISR_NOERRCODE 7     ; Device Not Available
ISR_ERRCODE   8     ; Double Fault (mit Error Code!)
ISR_NOERRCODE 9     ; Coprocessor Segment Overrun
ISR_ERRCODE   10    ; Invalid TSS (mit Error Code!)
ISR_ERRCODE   11    ; Segment Not Present (mit Error Code!)
ISR_ERRCODE   12    ; Stack Fault (mit Error Code!)
ISR_ERRCODE   13    ; General Protection Fault (mit Error Code!)
ISR_ERRCODE   14    ; Page Fault (mit Error Code!)
ISR_NOERRCODE 15    ; Reserved
ISR_NOERRCODE 16    ; x87 FPU Error
ISR_ERRCODE   17    ; Alignment Check (mit Error Code!)
ISR_NOERRCODE 18    ; Machine Check
ISR_NOERRCODE 19    ; SIMD Floating-Point Exception
ISR_NOERRCODE 20    ; Reserved
ISR_NOERRCODE 21    ; Reserved
ISR_NOERRCODE 22    ; Reserved
ISR_NOERRCODE 23    ; Reserved
ISR_NOERRCODE 24    ; Reserved
ISR_NOERRCODE 25    ; Reserved
ISR_NOERRCODE 26    ; Reserved
ISR_NOERRCODE 27    ; Reserved
ISR_NOERRCODE 28    ; Reserved
ISR_NOERRCODE 29    ; Reserved
ISR_ERRCODE   30    ; Security Exception (mit Error Code!)
ISR_NOERRCODE 31    ; Reserved

; IRQ-Handler (32-47)
IRQ 0, 32     ; Timer (IRQ0)
IRQ 1, 33     ; Keyboard (IRQ1)
IRQ 2, 34     ; Cascade
IRQ 3, 35     ; COM2
IRQ 4, 36     ; COM1
IRQ 5, 37     ; LPT2
IRQ 6, 38     ; Floppy
IRQ 7, 39     ; LPT1
IRQ 8, 40     ; RTC
IRQ 9, 41     ; Free
IRQ 10, 42    ; Free
IRQ 11, 43    ; Free
IRQ 12, 44    ; PS/2 Mouse
IRQ 13, 45    ; FPU
IRQ 14, 46    ; Primary ATA
IRQ 15, 47    ; Secondary ATA

; Gemeinsamer ISR-Stub
; Stack-Layout beim Eintritt:
;   [rsp+0]  = Interrupt-Nummer
;   [rsp+8]  = Error Code (oder 0)
;   [rsp+16] = RIP (von CPU gepusht)
;   [rsp+24] = CS  (von CPU gepusht)
;   [rsp+32] = RFLAGS (von CPU gepusht)
;   [rsp+40] = RSP (von CPU gepusht, falls Privilege-Level wechselt)
;   [rsp+48] = SS  (von CPU gepusht, falls Privilege-Level wechselt)
isr_common_stub:
    ; Register sichern
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; Segment-Register sichern
    mov ax, ds
    push rax
    mov ax, es
    push rax
    mov ax, fs
    push rax
    mov ax, gs
    push rax

    ; Kernel Data Segment laden
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; C-Handler aufrufen (RSP zeigt auf gesicherte Register)
    mov rdi, rsp    ; Pointer auf Register-Frame als Argument
    call isr_handler

    ; Segment-Register wiederherstellen
    pop rax
    mov gs, ax
    pop rax
    mov fs, ax
    pop rax
    mov es, ax
    pop rax
    mov ds, ax

    ; Register wiederherstellen
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    ; Error Code und Interrupt-Nummer vom Stack entfernen
    add rsp, 16

    ; Interrupt Return
    iretq

; Gemeinsamer IRQ-Stub
irq_common_stub:
    ; Register sichern (gleich wie ISR)
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; Segment-Register sichern
    mov ax, ds
    push rax
    mov ax, es
    push rax
    mov ax, fs
    push rax
    mov ax, gs
    push rax

    ; Kernel Data Segment laden
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; C-Handler aufrufen
    mov rdi, rsp    ; Pointer auf Register-Frame
    call irq_handler

    ; Segment-Register wiederherstellen
    pop rax
    mov gs, ax
    pop rax
    mov fs, ax
    pop rax
    mov es, ax
    pop rax
    mov ds, ax

    ; Register wiederherstellen
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    ; Error Code und IRQ-Nummer entfernen
    add rsp, 16

    ; Interrupt Return
    iretq
