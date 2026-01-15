; =============================================================================
; KiOS Stage2 - Gefixte Version (Final)
; =============================================================================

[BITS 16]
[ORG 0x7E00]

; -----------------------------------------------------------------------------
; WICHTIG: Entry Point Jump
; Wir springen über die Daten direkt zum Code. Ohne das führt die CPU
; 'kernel_lba_current' als Befehl aus -> Crash.
; -----------------------------------------------------------------------------
jmp stage2_start

; -----------------------------------------------------------------------------
; Konstanten
; -----------------------------------------------------------------------------
KERNEL_PHYS_ADDR    equ 0x100000
KERNEL_VIRT_ADDR    equ 0x100000

; Kernel Start (LBA) und Sektoren
kernel_lba_start  equ 34   ; Start-LBA des Kernels
kernel_sectors    equ 73   ; Anzahl der Kernel-Sektoren

; GDT Offsets
GDT_NULL_SEG        equ 0x00
GDT_CODE32_SEG      equ 0x08
GDT_DATA32_SEG      equ 0x10
GDT_CODE64_SEG      equ 0x18
GDT_DATA64_SEG      equ 0x20

; -----------------------------------------------------------------------------
; Globale Variablen
; -----------------------------------------------------------------------------
boot_drive         db 0                   ; Hier speichern wir die Drive ID vom BIOS
kernel_lba_current dw kernel_lba_start

; WICHTIG: Alignment für das DAP. Manche BIOS/QEMU Versionen mögen es nicht,
; wenn das DAP auf einer "krummen" Adresse liegt.
align 4
dap:
    times 16 db 0

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

; -----------------------------------------------------------------------------
; 16-Bit Hilfsfunktionen
; -----------------------------------------------------------------------------
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

enable_a20:
    in al, 0x92
    test al, 2
    jnz .a20_done
    or al, 2
    and al, 0xFE
    out 0x92, al
.a20_done:
    ret

; -----------------------------------------------------------------------------
; Fallback Memory Map (64-Bit)
; -----------------------------------------------------------------------------
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

; -----------------------------------------------------------------------------
; Kernel Laden (16-Bit BIOS, Chunked, safe)
; -----------------------------------------------------------------------------
load_kernel_16:
    pusha
    
    ; Disk Controller Reset (hilft oft bei realer Hardware/striktem QEMU)
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

; -----------------------------------------------------------------------------
; 32-Bit Protected Mode
; -----------------------------------------------------------------------------
[BITS 32]
protected_mode_entry:
    mov ax, GDT_DATA32_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    ; Kernel von 0x80000 nach 0x100000 kopieren
    mov esi, 0x80000
    mov edi, KERNEL_PHYS_ADDR
    mov ecx, kernel_sectors
    shl ecx, 9
    shr ecx, 2
    rep movsd

    ; Paging Setup
    call setup_paging

    ; Long Mode Enable
    mov eax, cr4
    or eax, (1 << 5)
    mov cr4, eax
    mov eax, pml4_table
    mov cr3, eax
    mov ecx, 0xC0000080
    rdmsr
    or eax, (1 << 8)
    wrmsr
    mov eax, cr0
    or eax, (1 << 31)
    mov cr0, eax

    jmp GDT_CODE64_SEG:long_mode_entry

setup_paging:
    ; Tables nullen (PML4, PDPT und PD liegen hintereinander bei dir)
    mov edi, pml4_table
    xor eax, eax
    mov ecx, 4096 * 3 / 4 
    rep stosd

    ; PML4 -> PDPT
    mov eax, pdpt_table
    or eax, 0b11      ; Present + Writable
    mov [pml4_table], eax

    ; PDPT -> PD
    mov eax, pd_table
    or eax, 0b11      ; Present + Writable
    mov [pdpt_table], eax

    ; PD füllen: Wir mappen die ersten 10 MB als 2MB Pages (Identity)
    ; Das reicht für Bootloader + Kernel + Stack
    mov edi, pd_table
    mov eax, 0b10000011 ; Present + Writable + PS (2MB Page)
    mov ecx, 512        ; 512 * 2MB = 1GB
.fill_pd:
    mov [edi], eax
    add eax, 0x200000
    add edi, 8
    loop .fill_pd
    ret

[BITS 64]
long_mode_entry:
    ; Segmente neu laden für 64-bit
    mov ax, GDT_DATA64_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; WICHTIG: Stack-Pointer auf eine sichere Adresse setzen (z.B. 0x90000)
    ; bevor wir in den Kernel springen
    mov rsp, 0x90000 

    ; Kernel-Einstiegspunkt (0x100000)
    mov rax, 0x100000
    
    ; Falls der Kernel zurückkehrt -> Hängenbleiben
    call rax
.halt:
    cli
    hlt
    jmp .halt

; -----------------------------------------------------------------------------
; GDT
; -----------------------------------------------------------------------------
align 16
gdt_start:
gdt_null:
    dq 0
gdt_code32:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00
gdt_data32:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00
gdt_code64:
    dw 0x0000
    dw 0x0000
    db 0x00
    db 10011010b
    db 00100000b
    db 0x00
gdt_data64:
    dw 0x0000
    dw 0x0000
    db 0x00
    db 10010010b
    db 00000000b
    db 0x00
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; -----------------------------------------------------------------------------
; Strings
; -----------------------------------------------------------------------------
msg_stage2:     db "KiOS Stage2", 13, 10, 0
msg_a20:        db "  Enabling A20... ", 0
msg_load_kernel:db "  Loading kernel... ", 0
msg_pm:         db "  Entering Protected Mode...", 13, 10, 0
msg_ok:         db "OK", 13, 10, 0
msg_disk_error: db "DISK ERROR!", 13, 10, 0
msg_e820_done:  db "  Memory Map done.", 13, 10, 0

; -----------------------------------------------------------------------------
; Page Tables
; -----------------------------------------------------------------------------
align 4096
pml4_table:
    times 4096 db 0
pdpt_table:
    times 4096 db 0
pd_table:
    times 4096 db 0

; -----------------------------------------------------------------------------
; Padding
; -----------------------------------------------------------------------------
times 16384 - ($ - $$) db 0