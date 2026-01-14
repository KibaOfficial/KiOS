; =============================================================================
; KiOS Bootloader - Stage 2
; =============================================================================
; Aufgabe: 
;   1. A20 Gate aktivieren (Zugriff auf > 1MB Speicher)
;   2. Memory Map per E820 auslesen
;   3. Kernel von Disk laden (an 0x100000 = 1MB)
;   4. GDT laden
;   5. In Protected Mode wechseln (32-Bit)
;   6. Paging aufsetzen (nötig für 64-Bit)
;   7. In Long Mode wechseln (64-Bit)
;   8. Zum C Kernel springen
; =============================================================================

[BITS 16]
[ORG 0x7E00]                    ; Wir werden an 0x7E00 geladen (nach Stage 1)

; =============================================================================
; Konstanten
; =============================================================================
KERNEL_PHYS_ADDR    equ 0x100000    ; Kernel wird bei 1MB geladen
KERNEL_VIRT_ADDR    equ 0x100000    ; Virtuelle Adresse = Physische (Identity Map)

; GDT Segment Selektoren (Offset in GDT)
GDT_NULL_SEG        equ 0x00
GDT_CODE32_SEG      equ 0x08        ; 32-Bit Code Segment
GDT_DATA32_SEG      equ 0x10        ; 32-Bit Data Segment
GDT_CODE64_SEG      equ 0x18        ; 64-Bit Code Segment
GDT_DATA64_SEG      equ 0x20        ; 64-Bit Data Segment

; =============================================================================
; Entry Point (16-Bit Real Mode)
; =============================================================================
stage2_start:
    ; Segmentregister aufsetzen
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00              ; Stack unter Stage 1

    ; Willkommensnachricht
    mov si, msg_stage2
    call print_string_16

    ; ─────────────────────────────────────────────────────────────────────────
    ; Schritt 1: A20 Gate aktivieren
    ; ─────────────────────────────────────────────────────────────────────────
    ; Das A20 Gate ist ein historisches Relikt vom 8086.
    ; Ohne A20 kann man nur 1MB adressieren (Adressen wrappen bei 1MB).
    ; Wir müssen es aktivieren für > 1MB Zugriff.
    
    mov si, msg_a20
    call print_string_16
    call enable_a20
    mov si, msg_ok
    call print_string_16

    ; ─────────────────────────────────────────────────────────────────────────
    ; Schritt 2: Memory Map per E820 auslesen
    ; ─────────────────────────────────────────────────────────────────────────
    ; TEMPORARILY DISABLED: E820 hangs on QEMU - using fallback
    call create_fallback_memory_map
    mov si, msg_e820_done
    call print_string_16

    ; ─────────────────────────────────────────────────────────────────────────
    ; Schritt 3: Kernel von Disk laden (an 0x100000 = 1MB)
    ; ─────────────────────────────────────────────────────────────────────────
    mov si, msg_load_kernel
    call print_string_16
    call load_kernel
    mov si, msg_ok
    call print_string_16

    ; ─────────────────────────────────────────────────────────────────────────
    ; Schritt 4: In Protected Mode wechseln (32-Bit)
    ; ─────────────────────────────────────────────────────────────────────────
    mov si, msg_pm
    call print_string_16

    cli                         ; Interrupts aus (WICHTIG vor Mode-Wechsel)

    lgdt [gdt_descriptor]       ; GDT laden
                                ; LGDT = Load Global Descriptor Table

    ; CR0 Register modifizieren: PE Bit (Protection Enable) setzen
    mov eax, cr0
    or eax, 1                   ; Bit 0 = PE
    mov cr0, eax

    ; Far Jump zu 32-Bit Code
    ; Das flusht die Instruction Pipeline und lädt CS mit neuem Segment
    jmp GDT_CODE32_SEG:protected_mode_entry

; =============================================================================
; 16-Bit Hilfsfunktionen
; =============================================================================

; Gibt String aus (SI = Pointer, null-terminiert)
print_string_16:
    pusha
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp .loop
.done:
    popa
    ret

; A20 Gate aktivieren (Fast A20 Methode)
enable_a20:
    ; Methode 1: Fast A20 über Port 0x92
    in al, 0x92                 ; Lese System Control Port A
    test al, 2                  ; Ist A20 schon aktiv?
    jnz .a20_done               ; Ja → fertig
    or al, 2                    ; Bit 1 setzen = A20 aktivieren
    and al, 0xFE                ; Bit 0 löschen (Reset verhindern!)
    out 0x92, al                ; Zurückschreiben
.a20_done:
    ret

; ─────────────────────────────────────────────────────────────────────────
; E820 Memory Map auslesen
; Ergebnis:
; [0x10000] = Anzahl der Einträge (WORD)
; [0x10004..] = E820 Einträge (je 24 Bytes)
; ─────────────────────────────────────────────────────────────────────────
get_memory_map:
    push ax
    push bx
    push cx
    push dx
    push di
    push si
    push es

    ; Lineare Adressierung verwenden (ES = 0)
    ; Memory Map bei 0x10000 (64KB) speichern - safe, nach Stage2 Code
    xor ax, ax
    mov es, ax
    mov di, 0x10004         ; Entries starten bei 0x10004
    mov word [0x10000], 0   ; Entry-Counter bei 0x10000
    xor ebx, ebx            ; EBX = 0 (Continuation Value - FIRST call only)
    xor si, si              ; SI = Entry Counter

.next_e820:
    ; Safety check: Max 32 Entries
    cmp si, 32
    jae .done               ; Zu viele → abbrechen

    mov eax, 0xE820         ; Function code (BIOS might trash it)
    mov ecx, 24             ; Buffer size (BIOS might trash it)
    mov edx, 0x534D4150     ; 'SMAP' Magic (BIOS might trash it)
    int 0x15
    jc .done                ; Carry set = error or done

    ; EDX should still be 'SMAP' but some BIOSes trash it
    ; EAX should be 'SMAP' on success
    cmp eax, 0x534D4150
    jne .done               ; BIOS doesn't support E820

    ; CL should contain actual bytes stored (usually 20 or 24)
    ; We ignore this and assume 24

    ; Entry ist gültig - EBX enthält Continuation Value
    inc si                  ; Entry Counter erhöhen
    add di, 24              ; Nächster Entry (fixed 24 bytes)

    ; If EBX = 0, list is complete
    test ebx, ebx
    jz .done                ; EBX == 0 → fertig

    jmp .next_e820          ; Weiter mit nächstem Entry

.done:
    mov [0x10000], si       ; Anzahl der Entries speichern

    pop es
    pop si
    pop di
    pop dx
    pop cx
    pop bx
    pop ax
    ret

; ─────────────────────────────────────────────────────────────────────────
; Fallback Memory Map (hardcoded für 128MB RAM)
; Erstellt eine simple Memory Map bei 0x10000 wenn E820 nicht funktioniert
; ─────────────────────────────────────────────────────────────────────────
create_fallback_memory_map:
    push ax
    push cx
    push di
    push es

    ; Use segment 0x1000 so we can access 0x10000+ addresses
    mov ax, 0x1000
    mov es, ax
    mov di, 0x0004          ; ES:DI = 0x1000:0x0004 = 0x10004

    ; Entry 0: 0x0000 - 0x9FFFF (640KB) = FREE
    ; Base address (8 bytes)
    mov word [es:di], 0x0000        ; Base low word
    mov word [es:di+2], 0x0000      ; Base low high word
    mov word [es:di+4], 0x0000      ; Base high word
    mov word [es:di+6], 0x0000      ; Base high high word
    ; Length (8 bytes)
    mov word [es:di+8], 0x0000      ; Length low word (640KB = 0xA0000)
    mov word [es:di+10], 0x000A     ; Length low high word
    mov word [es:di+12], 0x0000     ; Length high word
    mov word [es:di+14], 0x0000     ; Length high high word
    ; Type (4 bytes)
    mov word [es:di+16], 0x0001     ; Type = FREE
    mov word [es:di+18], 0x0000
    add di, 24

    ; Entry 1: 0x100000 - 0x7FFFFFF (127MB) = FREE
    ; Base address (8 bytes)
    mov word [es:di], 0x0000        ; Base low word
    mov word [es:di+2], 0x0010      ; Base low high word (1MB = 0x100000)
    mov word [es:di+4], 0x0000      ; Base high word
    mov word [es:di+6], 0x0000      ; Base high high word
    ; Length (8 bytes)
    mov word [es:di+8], 0x0000      ; Length low word (127MB = 0x7F00000)
    mov word [es:di+10], 0x7F00     ; Length low high word
    mov word [es:di+12], 0x0000     ; Length high word
    mov word [es:di+14], 0x0000     ; Length high high word
    ; Type (4 bytes)
    mov word [es:di+16], 0x0001     ; Type = FREE
    mov word [es:di+18], 0x0000

    ; Anzahl der Entries = 2 bei 0x10000 (ES:0x0000)
    mov word [es:0x0000], 2

    pop es
    pop di
    pop cx
    pop ax
    ret

; Kernel von Disk laden
; Wir benutzen BIOS INT 13h Extended Read (LBA Mode)
load_kernel:
    pusha
    
    ; Wir laden erstmal in einen Buffer unter 1MB (0x10000)
    ; und kopieren dann später in Protected Mode nach 0x100000
    
    mov ah, 0x42                ; Extended Read
    mov dl, [0x7C00 + 510 - 2]  ; Boot drive (gespeichert in Stage 1... 
                                ; eigentlich sollten wir das besser übergeben)
    mov dl, 0x80                ; Erstmal hardcoded: erste Festplatte
    mov si, dap                 ; Disk Address Packet
    int 0x13
    jc .disk_error
    
    popa
    ret

.disk_error:
    mov si, msg_disk_error
    call print_string_16
    cli
    hlt

; Disk Address Packet für INT 13h Extended Read
dap:
    db 0x10                     ; Größe des DAP (16 Bytes)
    db 0                        ; Reserviert
    dw 64                       ; Anzahl Sektoren (64 * 512 = 32KB)
    dw 0x0000                   ; Offset (Ziel)
    dw 0x1000                   ; Segment (Ziel) → 0x1000:0x0000 = 0x10000
    dq 34                       ; LBA Start-Sektor (nach Stage1 + Stage2)
                                ; Sektor 0 = Stage1, 1-33 = Stage2, 34+ = Kernel

; =============================================================================
; 32-Bit Protected Mode Code
; =============================================================================
[BITS 32]

protected_mode_entry:
    ; ─────────────────────────────────────────────────────────────────────────
    ; Segmentregister mit Data Segment laden
    ; ─────────────────────────────────────────────────────────────────────────
    mov ax, GDT_DATA32_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000            ; Stack bei 576KB (sicherer Bereich)

    ; ─────────────────────────────────────────────────────────────────────────
    ; Kernel nach 0x100000 (1MB) kopieren
    ; ─────────────────────────────────────────────────────────────────────────
    ; Quelle: 0x10000 (wo BIOS den Kernel geladen hat)
    ; Ziel: 0x100000 (1MB, wo der Kernel laufen soll)
    
    mov esi, 0x10000            ; Quelle
    mov edi, 0x100000           ; Ziel
    mov ecx, 32768 / 4          ; 32KB / 4 = Anzahl DWORDs
    rep movsd                   ; Kopiere ECX DWORDs von ESI nach EDI

    ; ─────────────────────────────────────────────────────────────────────────
    ; Schritt 4: Paging aufsetzen
    ; ─────────────────────────────────────────────────────────────────────────
    ; Für Long Mode (64-Bit) ist Paging PFLICHT.
    ; Wir erstellen eine einfache Identity-Mapping Page Table:
    ; Virtuelle Adresse = Physische Adresse (für die ersten 2GB)
    
    call setup_paging

    ; ─────────────────────────────────────────────────────────────────────────
    ; Schritt 5: In Long Mode wechseln (64-Bit)
    ; ─────────────────────────────────────────────────────────────────────────
    
    ; PAE (Physical Address Extension) aktivieren
    mov eax, cr4
    or eax, (1 << 5)            ; Bit 5 = PAE
    mov cr4, eax

    ; Page Table Basis-Adresse in CR3 laden
    mov eax, pml4_table
    mov cr3, eax

    ; Long Mode Enable in EFER MSR
    ; MSR = Model Specific Register, Zugriff über RDMSR/WRMSR
    mov ecx, 0xC0000080         ; EFER MSR Nummer
    rdmsr                       ; Lese MSR → EDX:EAX
    or eax, (1 << 8)            ; Bit 8 = LME (Long Mode Enable)
    wrmsr                       ; Schreibe MSR

    ; Paging aktivieren (und damit Long Mode!)
    mov eax, cr0
    or eax, (1 << 31)           ; Bit 31 = PG (Paging)
    mov cr0, eax

    ; Jetzt sind wir in "Compatibility Mode" (32-Bit in Long Mode)
    ; Wir brauchen einen Far Jump mit 64-Bit Code Segment

    jmp GDT_CODE64_SEG:long_mode_entry

; =============================================================================
; Paging Setup
; =============================================================================
setup_paging:
    ; ─────────────────────────────────────────────────────────────────────────
    ; Page Tables auf 0 initialisieren
    ; ─────────────────────────────────────────────────────────────────────────
    mov edi, pml4_table
    xor eax, eax
    mov ecx, 4096               ; 4 Pages à 4KB = 16KB
    rep stosd                   ; Fülle mit 0

    ; ─────────────────────────────────────────────────────────────────────────
    ; Page Tables aufbauen
    ; ─────────────────────────────────────────────────────────────────────────
    ; Struktur (64-Bit Paging mit 2MB Pages):
    ;
    ; PML4 (Page Map Level 4) - 512 Einträge, je 512GB
    ;   └→ PDPT (Page Directory Pointer Table) - 512 Einträge, je 1GB
    ;        └→ PD (Page Directory) - 512 Einträge, je 2MB
    ;             └→ [Wir nutzen 2MB Pages, keine Page Table nötig]
    ;
    ; Flags in jedem Eintrag:
    ;   Bit 0: Present (1 = gültig)
    ;   Bit 1: Read/Write (1 = schreibbar)
    ;   Bit 2: User/Supervisor (0 = nur Ring 0)
    ;   Bit 7: Page Size (1 = 2MB Page in PD, 1GB in PDPT)

    ; PML4[0] → PDPT
    mov eax, pdpt_table
    or eax, 0b11                ; Present + Writable
    mov [pml4_table], eax

    ; PDPT[0] → PD
    mov eax, pd_table
    or eax, 0b11                ; Present + Writable
    mov [pdpt_table], eax

    ; PD: 512 Einträge für 512 * 2MB = 1GB Identity Mapping
    mov edi, pd_table
    mov eax, 0b10000011         ; Present + Writable + Page Size (2MB)
    mov ecx, 512

.fill_pd:
    mov [edi], eax
    add eax, 0x200000           ; Nächste 2MB
    add edi, 8                  ; Nächster Eintrag (8 Bytes in 64-Bit)
    loop .fill_pd

    ret

; =============================================================================
; 64-Bit Long Mode Code
; =============================================================================
[BITS 64]

long_mode_entry:
    ; ─────────────────────────────────────────────────────────────────────────
    ; Segmentregister in Long Mode
    ; ─────────────────────────────────────────────────────────────────────────
    ; In Long Mode werden DS, ES, FS, GS, SS ignoriert (flat memory model)
    ; Aber wir setzen sie trotzdem auf gültige Werte
    
    mov ax, GDT_DATA64_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Stack bei 2MB (genug Platz)
    mov rsp, 0x200000

    ; ─────────────────────────────────────────────────────────────────────────
    ; Springe zum C Kernel!
    ; ─────────────────────────────────────────────────────────────────────────
    ; Der Kernel wurde nach 0x100000 (1MB) kopiert
    ; kernel_entry.asm ist am Anfang und ruft kernel_main() auf
    
    mov rax, 0x100000
    jmp rax

    ; Falls Kernel returned (sollte nie passieren)
.halt:
    cli
    hlt
    jmp .halt

; =============================================================================
; GDT (Global Descriptor Table)
; =============================================================================
; Die GDT definiert Speichersegmente. In Long Mode sind die meisten Felder
; ignoriert, aber wir brauchen trotzdem gültige Code/Data Segmente.

align 16
gdt_start:

; Null Descriptor (muss immer erster Eintrag sein)
gdt_null:
    dq 0

; 32-Bit Code Segment (für Protected Mode Übergang)
gdt_code32:
    dw 0xFFFF               ; Limit 0-15
    dw 0x0000               ; Base 0-15
    db 0x00                 ; Base 16-23
    db 10011010b            ; Access: P=1, DPL=0, S=1, E=1, DC=0, RW=1, A=0
    db 11001111b            ; Flags: G=1, D=1, L=0, Reserved + Limit 16-19
    db 0x00                 ; Base 24-31

; 32-Bit Data Segment
gdt_data32:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b            ; Access: P=1, DPL=0, S=1, E=0, DC=0, RW=1, A=0
    db 11001111b
    db 0x00

; 64-Bit Code Segment
gdt_code64:
    dw 0x0000               ; Limit (ignoriert in Long Mode)
    dw 0x0000               ; Base (ignoriert)
    db 0x00                 ; Base (ignoriert)
    db 10011010b            ; Access: P=1, DPL=0, S=1, E=1, DC=0, RW=1, A=0
    db 00100000b            ; Flags: G=0, D=0, L=1 (Long Mode!), Reserved
    db 0x00                 ; Base (ignoriert)

; 64-Bit Data Segment
gdt_data64:
    dw 0x0000
    dw 0x0000
    db 0x00
    db 10010010b            ; Access: P=1, DPL=0, S=1, E=0, DC=0, RW=1, A=0
    db 00000000b
    db 0x00

gdt_end:

; GDT Descriptor (wird mit LGDT geladen)
gdt_descriptor:
    dw gdt_end - gdt_start - 1      ; Größe - 1
    dd gdt_start                     ; Adresse (32-Bit, wird in PM erweitert)

; =============================================================================
; Strings
; =============================================================================
msg_stage2:     db "KiOS Stage2", 13, 10, 0
msg_a20:        db "  Enabling A20... ", 0
msg_load_kernel:db "  Loading kernel... ", 0
msg_pm:         db "  Entering Protected Mode...", 13, 10, 0
msg_ok:         db "OK", 13, 10, 0
msg_disk_error: db "DISK ERROR!", 13, 10, 0
msg_e820_done:  db "  Memory Map done.", 13, 10, 0

; =============================================================================
; Page Tables
; =============================================================================
; Müssen 4KB-aligned sein!

align 4096
pml4_table:
    times 4096 db 0

pdpt_table:
    times 4096 db 0

pd_table:
    times 4096 db 0

; =============================================================================
; Padding
; =============================================================================
; Stage 2 muss auf volle Sektoren gepaddet werden
; Wir reservieren 16KB (32 Sektoren)

times 16384 - ($ - $$) db 0