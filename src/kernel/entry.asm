; =============================================================================
; KiOS Kernel Entry Point
; =============================================================================
; Dies ist die Brücke zwischen Bootloader (Assembly) und Kernel (C).
; Wird an 0x100000 (1MB) geladen und aufgerufen.
; =============================================================================

[BITS 64]

; Externe Symbole (kommen aus C)
extern kernel_main

; Globale Symbole (für Linker sichtbar)
global _start
global _kernel_start

section .text

; =============================================================================
; Entry Point
; =============================================================================
_start:
_kernel_start:
    ; ─────────────────────────────────────────────────────────────────────────
    ; WICHTIG: Interrupts deaktivieren!
    ; IDT und PIC werden von C-Code initialisiert
    ; ─────────────────────────────────────────────────────────────────────────
    cli

    ; Stack neu setzen (sicher)
    mov rsp, 0x200000

    ; Bildschirm komplett löschen (80x25 * 2 = 4000 Bytes)
    mov rdi, 0xB8000            ; VGA Buffer Adresse
    mov rax, 0x0720072007200720 ; 4x Space mit Attribut 0x07 (hellgrau auf schwarz)
    mov rcx, 500                ; 4000 / 8 = 500 QWORDs
    rep stosq                   ; Fülle den Buffer (64-bit writes)

    ; VGA Cursor auf Position 0,0 setzen
    mov dx, 0x3D4
    mov al, 0x0F                ; Cursor Low Register
    out dx, al
    mov dx, 0x3D5
    xor al, al                  ; Position 0
    out dx, al
    mov dx, 0x3D4
    mov al, 0x0E                ; Cursor High Register
    out dx, al
    mov dx, 0x3D5
    xor al, al                  ; Position 0
    out dx, al

    ; Debug: Testmuster in den VGA-Puffer schreiben
    mov rdi, 0xB8000            ; VGA Buffer Adresse
    mov rax, 0x4141414141414141 ; Muster: 'AAAA' mit Attribut 0x07
    mov rcx, 500                ; 4000 / 8 = 500 QWORDs
    rep stosq                   ; Fülle den Buffer (64-bit writes)

    ; Debug: Cursor auf Position 10,10 setzen
    mov dx, 0x3D4
    mov al, 0x0F                ; Cursor Low Register
    out dx, al
    mov dx, 0x3D5
    mov al, 10                  ; Spalte 10
    out dx, al
    mov dx, 0x3D4
    mov al, 0x0E                ; Cursor High Register
    out dx, al
    mov dx, 0x3D5
    mov al, 0                   ; Zeile 10
    out dx, al

    ; Debug: Fülle den gesamten VGA-Puffer mit einem Muster mit schwarzem Hintergrund
    mov rdi, 0xB8000            ; VGA Buffer Adresse
    mov rax, 0x0720072007200720 ; Muster: ' ' mit Attribut 0x07 (hellgrau auf schwarz)
    mov rcx, 500                ; 4000 / 8 = 500 QWORDs
    rep stosq                   ; Fülle den Buffer (64-bit writes)

    ; Debug: Setze den Cursor explizit auf (0, 0)
    mov dx, 0x3D4
    mov al, 0x0F                ; Cursor Low Register
    out dx, al
    mov dx, 0x3D5
    xor al, al                  ; Spalte 0
    out dx, al
    mov dx, 0x3D4
    mov al, 0x0E                ; Cursor High Register
    out dx, al
    mov dx, 0x3D5
    xor al, al                  ; Zeile 0
    out dx, al

    ; Bildschirm löschen und Cursor zurücksetzen
    mov rdi, 0xB8000            ; VGA Buffer Adresse
    mov rax, 0x0720072007200720 ; Muster: ' ' mit Attribut 0x07 (hellgrau auf schwarz)
    mov rcx, 500                ; 4000 / 8 = 500 QWORDs
    rep stosq                   ; Fülle den Buffer (64-bit writes)

    ; Cursor auf Position 0,0 setzen
    mov dx, 0x3D4
    mov al, 0x0F                ; Cursor Low Register
    out dx, al
    mov dx, 0x3D5
    xor al, al                  ; Spalte 0
    out dx, al
    mov dx, 0x3D4
    mov al, 0x0E                ; Cursor High Register
    out dx, al
    mov dx, 0x3D5
    xor al, al                  ; Zeile 0
    out dx, al

    ; Zähler inkrementieren
    inc qword [reboot_counter]

    ; Zähler im VGA-Puffer anzeigen
    mov rsi, reboot_counter
    mov rax, [rsi]             ; Lade den Zählerwert
    mov rdi, 0xB8000           ; VGA-Puffer-Adresse
    mov rcx, 20                ; Maximal 20 Zeichen
    call print_number_to_vga   ; Schreibe die Zahl auf den Bildschirm

    ; C Kernel aufrufen
    ; In System V AMD64 ABI: Argumente in RDI, RSI, RDX, RCX, R8, R9
    ; Wir übergeben nichts, also keine Argumente setzen
    call kernel_main

    ; Falls kernel_main returned (sollte nie passieren)
.halt:
    cli                         ; Interrupts aus
    hlt                         ; CPU anhalten
    jmp .halt                   ; Falls ein NMI aufweckt

; =============================================================================
; Interrupt Service Routines (Stubs für später)
; =============================================================================
; Diese werden später für echte Interrupt-Behandlung gebraucht

global isr_stub_table

; Macro für ISR ohne Error Code
%macro isr_no_err 1
global isr_stub_%1
isr_stub_%1:
    push 0                      ; Dummy Error Code
    push %1                     ; Interrupt Nummer
    jmp isr_common
%endmacro

; Macro für ISR mit Error Code (CPU pusht Error Code automatisch)
%macro isr_err 1
global isr_stub_%1
isr_stub_%1:
    push %1                     ; Interrupt Nummer
    jmp isr_common
%endmacro

; CPU Exceptions (0-31)
isr_no_err 0                    ; Division by Zero
isr_no_err 1                    ; Debug
isr_no_err 2                    ; NMI
isr_no_err 3                    ; Breakpoint
isr_no_err 4                    ; Overflow
isr_no_err 5                    ; Bound Range Exceeded
isr_no_err 6                    ; Invalid Opcode
isr_no_err 7                    ; Device Not Available
isr_err    8                    ; Double Fault (hat Error Code)
isr_no_err 9                    ; Coprocessor Segment Overrun
isr_err    10                   ; Invalid TSS
isr_err    11                   ; Segment Not Present
isr_err    12                   ; Stack-Segment Fault
isr_err    13                   ; General Protection Fault
isr_err    14                   ; Page Fault
isr_no_err 15                   ; Reserved
isr_no_err 16                   ; x87 Floating-Point Exception
isr_err    17                   ; Alignment Check
isr_no_err 18                   ; Machine Check
isr_no_err 19                   ; SIMD Floating-Point Exception
isr_no_err 20                   ; Virtualization Exception
isr_no_err 21                   ; Reserved
isr_no_err 22                   ; Reserved
isr_no_err 23                   ; Reserved
isr_no_err 24                   ; Reserved
isr_no_err 25                   ; Reserved
isr_no_err 26                   ; Reserved
isr_no_err 27                   ; Reserved
isr_no_err 28                   ; Reserved
isr_no_err 29                   ; Reserved
isr_err    30                   ; Security Exception
isr_no_err 31                   ; Reserved

; IRQs (32-47) - Hardware Interrupts
%assign i 32
%rep 16
isr_no_err i
%assign i i+1
%endrep

; Gemeinsamer ISR Handler
extern isr_handler

isr_common:
    ; Alle Register sichern
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

    ; C Handler aufrufen
    mov rdi, rsp                ; Pointer zum Stack Frame als Argument
    call isr_handler

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

    add rsp, 16                 ; Error Code + Interrupt Nummer entfernen
    iretq                       ; Interrupt Return (64-Bit)

; ISR Stub Table (Array von Funktionspointern)
section .data
isr_stub_table:
%assign i 0
%rep 48
    dq isr_stub_%+i
%assign i i+1
%endrep

reboot_counter dq 0

section .text

print_number_to_vga:
    ; Konvertiere Zahl in ASCII und schreibe sie in den VGA-Puffer
    ; (Implementierung hier einfügen)
    ret