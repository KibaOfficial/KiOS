; =============================================================================
; KiOS Bootloader - Stage 2
; =============================================================================
; Aufgabe:
;   1. A20 Gate aktivieren (Zugriff auf > 1MB Speicher)
;   2. Memory Map per E820 auslesen (oder Fallback verwenden)
;   3. Kernel von Disk laden (an 0x100000 = 1MB, chunked loading)
;   4. GDT laden
;   5. In Protected Mode wechseln (32-Bit)
;   6. Paging aufsetzen (nötig für 64-Bit)
;   7. In Long Mode wechseln (64-Bit)
;   8. Zum C Kernel springen
;
; Änderungen gegenüber v0.2.0:
;   - Dynamische Kernel-Sektor-Berechnung via Makefile
;   - Entry Point Jump um Daten-als-Code-Bug zu vermeiden
;   - Chunked Loading (16 Sektoren pro BIOS-Call)
;   - Disk Reset für bessere Hardware-Kompatibilität
;   - Boot Drive wird aus DL gespeichert statt hardcoded 0x80
;   - Höherer Load-Buffer (0x80000 statt 0x10000)
; =============================================================================

[BITS 16]
[ORG 0x7E00]                    ; Wir werden an 0x7E00 geladen (nach Stage 1)

; -----------------------------------------------------------------------------
; WICHTIG: Entry Point Jump
; -----------------------------------------------------------------------------
; Wir springen über die Daten direkt zum Code. Ohne das führt die CPU
; 'kernel_lba_current' als Befehl aus -> Crash.
; Dies ist ein Bugfix gegenüber v0.2.0, wo Konstanten direkt am Anfang standen.
jmp stage2_start

; =============================================================================
; Konstanten
; =============================================================================
KERNEL_PHYS_ADDR    equ 0x100000    ; Kernel wird bei 1MB geladen
KERNEL_VIRT_ADDR    equ 0x100000    ; Virtuelle Adresse = Physische (Identity Map)

; Kernel Start (LBA) und Sektoren
kernel_lba_start  equ 34            ; Start-LBA des Kernels (nach Stage 1+2)
                                    ; Sektor 0 = Stage1, 1-33 = Stage2, 34+ = Kernel

; Dynamische Sektoranzahl vom Makefile (via nasm -D_KERNEL_SECTORS=...)
; INNOVATION v0.3.0: Makefile berechnet automatisch die Kernel-Größe!
; Befehl: nasm -D_KERNEL_SECTORS=$(shell stat -c%s kernel.bin | awk ...)
%ifndef _KERNEL_SECTORS
    %define _KERNEL_SECTORS 80      ; Fallback falls nicht definiert
%endif
kernel_sectors    equ _KERNEL_SECTORS

; GDT Segment Selektoren (Offset in GDT)
GDT_NULL_SEG        equ 0x00
GDT_CODE32_SEG      equ 0x08        ; 32-Bit Code Segment
GDT_DATA32_SEG      equ 0x10        ; 32-Bit Data Segment
GDT_CODE64_SEG      equ 0x18        ; 64-Bit Code Segment
GDT_DATA64_SEG      equ 0x20        ; 64-Bit Data Segment

; =============================================================================
; Globale Variablen
; =============================================================================
boot_drive         db 0                   ; Boot Drive ID (z.B. 0x80 für HDD, 0x00 für Floppy)
                                          ; Wird vom BIOS in DL übergeben und hier gespeichert
kernel_lba_current dw kernel_lba_start    ; Aktueller LBA-Sektor beim Laden

; DAP (Disk Address Packet) für INT 13h Extended Read
; WICHTIG: Alignment für das DAP. Manche BIOS/QEMU Versionen mögen es nicht,
; wenn das DAP auf einer "krummen" Adresse liegt.
align 4
dap:
    times 16 db 0                         ; Wird zur Laufzeit gefüllt

; -----------------------------------------------------------------------------
; Stage 2 Entry Point
; -----------------------------------------------------------------------------
stage2_start:
    ; WICHTIG: Sofort DL (Boot Drive ID) retten!
    ; Das BIOS hat die ID (z.B. 0x80 für HDD, 0x00 für Floppy) in DL übergeben.
    mov [boot_drive], dl

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov si, msg_stage2
    call print_string_16

    ; A20 aktivieren
    mov si, msg_a20
    call print_string_16
    call enable_a20
    mov si, msg_ok
    call print_string_16

    ; Memory Map
    call create_fallback_memory_map
    mov si, msg_e820_done
    call print_string_16

    ; Kernel Laden (16-Bit BIOS)
    mov si, msg_load_kernel
    call print_string_16
    call load_kernel_16
    mov si, msg_ok
    call print_string_16

    ; Protected Mode
    mov si, msg_pm
    call print_string_16
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp GDT_CODE32_SEG:protected_mode_entry

; =============================================================================
; 16-Bit Hilfsfunktionen
; =============================================================================

; ─────────────────────────────────────────────────────────────────────────────
; print_string_16
; Gibt String aus (SI = Pointer, null-terminiert)
; ─────────────────────────────────────────────────────────────────────────────
print_string_16:
    pusha
.loop:
    lodsb                       ; Lade Byte von [SI] nach AL, erhöhe SI
    test al, al                 ; Ist AL == 0? (Null-Terminator)
    jz .done                    ; Ja → fertig
    mov ah, 0x0E                ; BIOS Teletype Output
    int 0x10                    ; BIOS Video Services
    jmp .loop
.done:
    popa
    ret

; ─────────────────────────────────────────────────────────────────────────────
; enable_a20
; A20 Gate aktivieren (Fast A20 Methode über Port 0x92)
; ─────────────────────────────────────────────────────────────────────────────
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

; ─────────────────────────────────────────────────────────────────────────────
; create_fallback_memory_map
; Fallback Memory Map (hardcoded für 128MB RAM)
; Erstellt eine simple Memory Map bei 0x10000 wenn E820 nicht funktioniert
;
; Ergebnis:
; [0x10000] = Anzahl der Einträge (WORD)
; [0x10002..] = E820 Einträge (je 24 Bytes)
; ─────────────────────────────────────────────────────────────────────────────
create_fallback_memory_map:
    push ax
    push di
    push es

    mov ax, 0x1000
    mov es, ax
    mov di, 2

    ; Entry 0: 0x00000000 - 0x0009FFFF (640KB)
    mov dword [es:di], 0x00000000
    mov dword [es:di+4], 0x00000000
    mov dword [es:di+8], 0x000A0000
    mov dword [es:di+12], 0x00000000
    mov dword [es:di+16], 1
    add di, 24

    ; Entry 1: 0x00100000 - 0x07FFFFFF (127MB)
    mov dword [es:di], 0x00100000
    mov dword [es:di+4], 0x00000000
    mov dword [es:di+8], 0x07F00000
    mov dword [es:di+12], 0x00000000
    mov dword [es:di+16], 1
    add di, 24

    mov word [es:0x0000], 2

    pop es
    pop di
    pop ax
    ret

; ─────────────────────────────────────────────────────────────────────────────
; load_kernel_16
; Kernel von Disk laden (16-Bit BIOS, Chunked Loading)
;
; INNOVATION v0.3.0: Chunked Loading!
; Wir laden den Kernel in Chunks von maximal 16 Sektoren.
; Das ist sicherer als große Loads auf einmal und verhindert BIOS-Timeouts.
;
; Load-Strategie:
;   1. Lade zu 0x80000 (512KB Bereich - mehr Platz als v0.2.0's 0x10000!)
;   2. In Protected Mode: Copy nach 0x100000 (1MB)
;
; Warum 0x80000 statt 0x10000?
;   - 0x10000 hat nur 64KB Platz bis zum nächsten kritischen Bereich
;   - 0x80000 hat ~448KB Platz → Kernel kann viel größer werden!
; ─────────────────────────────────────────────────────────────────────────────
load_kernel_16:
    pusha

    ; Disk Controller Reset (hilft oft bei realer Hardware/striktem QEMU)
    ; INT 13h AH=00h: Reset Disk System
    xor ax, ax
    mov dl, [boot_drive]
    int 0x13

    xor ax, ax
    mov ds, ax
    mov es, ax

    mov dl, [boot_drive]       
    mov cx, kernel_sectors     
    mov bx, 0x8000             ; Start-Segment (0x8000:0000 = 0x80000)
    mov di, 0                  ; Start-Offset im Segment

.load_next:
    cmp cx, 0
    je .done_loading

    ; Maximal 16 Sektoren pro Call (noch konservativer als 32)
    mov ax, cx
    cmp ax, 16
    jle .use_ax
    mov ax, 16
.use_ax:
    mov bp, ax

    ; DAP (Disk Address Packet) vorbereiten
    mov byte [dap + 0], 16      ; Size of DAP
    mov byte [dap + 1], 0       ; Reserved
    mov word [dap + 2], bp      ; Number of sectors
    mov word [dap + 4], di      ; Offset
    mov word [dap + 6], bx      ; Segment
    
    ; LBA (64-Bit) komplett initialisieren
    mov ax, [kernel_lba_current]
    mov [dap + 8], ax           ; LBA bits 0-15
    mov word [dap + 10], 0      ; LBA bits 16-31
    mov dword [dap + 12], 0     ; LBA bits 32-63 (WICHTIG: Null setzen!)

    mov si, dap
    mov ah, 0x42
    int 0x13
    jc .disk_error

    ; Nächste Adresse berechnen (Segment-Basis erhöhen)
    ; Jedes Mal wenn wir 'bp' Sektoren lesen, erhöhen wir das Segment um (bp * 512 / 16)
    ; Da bp=16 und 16*512/16 = 512, addieren wir einfach 0x200 zum Segment-Register
    mov ax, bp
    shl ax, 5                   ; ax = sectors * 512 / 16
    add bx, ax                  ; bx ist unser Segment-Pointer

    ; LBA hochzählen
    add word [kernel_lba_current], bp
    sub cx, bp
    jmp .load_next

.done_loading:
    popa
    ret

.disk_error:
    ; Fehlercode steht in AH - für Debugging hilfreich
    mov si, msg_disk_error
    call print_string_16
    cli
    hlt

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
    ; Quelle: 0x80000 (wo BIOS den Kernel geladen hat)
    ; Ziel: 0x100000 (1MB, wo der Kernel laufen soll)
    ; Größe: kernel_sectors * 512 Bytes (dynamisch berechnet!)

    mov esi, 0x80000            ; Quelle
    mov edi, KERNEL_PHYS_ADDR   ; Ziel (0x100000)
    mov ecx, kernel_sectors     ; Anzahl Sektoren
    shl ecx, 9                  ; * 512 (Bytes pro Sektor)
    shr ecx, 2                  ; / 4 (DWORDs statt Bytes)
    rep movsd                   ; Kopiere ECX DWORDs von ESI nach EDI

    ; ─────────────────────────────────────────────────────────────────────────
    ; Schritt 4: Paging aufsetzen
    ; ─────────────────────────────────────────────────────────────────────────
    ; Für Long Mode (64-Bit) ist Paging PFLICHT.
    ; Wir erstellen eine einfache Identity-Mapping Page Table:
    ; Virtuelle Adresse = Physische Adresse (für die ersten 1GB)

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
    ; Wir haben 3 Page Tables à 4KB = 12KB total
    mov edi, pml4_table
    xor eax, eax
    mov ecx, 4096 * 3 / 4       ; 12KB / 4 = 3072 DWORDs
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
    ; Das reicht für Bootloader + Kernel + Stack + PMM Bitmap + Heap
    mov edi, pd_table
    mov eax, 0b10000011         ; Present + Writable + Page Size (2MB)
    mov ecx, 512                ; 512 Einträge

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

    ; Stack bei 576KB (genug Platz, weit weg vom Kernel)
    mov rsp, 0x90000

    ; ─────────────────────────────────────────────────────────────────────────
    ; Springe zum C Kernel!
    ; ─────────────────────────────────────────────────────────────────────────
    ; Der Kernel wurde nach 0x100000 (1MB) kopiert
    ; entry.asm ist am Anfang und ruft kernel_main() auf

    mov rax, 0x100000
    call rax

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