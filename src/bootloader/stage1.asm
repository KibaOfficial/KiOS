; =============================================================================
; KiOS Bootloader - Stage 1
; =============================================================================
; Aufgabe: Stage 2 von Disk laden und dorthin springen
; Größe: Genau 512 Bytes (1 Sektor)
; Wird vom BIOS an 0x7C00 geladen
; =============================================================================

[BITS 16]                   ; 16-Bit Real Mode
[ORG 0x7C00]                ; BIOS lädt uns hierhin

; =============================================================================
; Konstanten
; =============================================================================
STAGE2_LOAD_SEG  equ 0x0000     ; Segment wo Stage 2 geladen wird
STAGE2_LOAD_OFF  equ 0x7E00     ; Offset (direkt nach uns: 0x7C00 + 512)
STAGE2_SECTORS   equ 32         ; Wieviele Sektoren laden (16KB sollte reichen)

; =============================================================================
; Entry Point
; =============================================================================
start:
    ; ─────────────────────────────────────────────────────────────────────────
    ; Schritt 1: Segmentregister auf 0 setzen für flache Adressierung
    ; ─────────────────────────────────────────────────────────────────────────
    xor ax, ax              ; AX = 0 (XOR mit sich selbst = 0)
    mov ds, ax              ; Data Segment = 0
    mov es, ax              ; Extra Segment = 0
    mov ss, ax              ; Stack Segment = 0
    mov sp, 0x7C00          ; Stack Pointer direkt unter unserem Code
                            ; Stack wächst nach UNTEN (zu kleineren Adressen)

    ; ─────────────────────────────────────────────────────────────────────────
    ; Schritt 2: Boot Drive Nummer speichern
    ; ─────────────────────────────────────────────────────────────────────────
    ; Das BIOS übergibt die Drive-Nummer in DL
    ; 0x00 = Floppy A, 0x80 = erste Festplatte
    mov [boot_drive], dl

    ; ─────────────────────────────────────────────────────────────────────────
    ; Schritt 3: Willkommensnachricht ausgeben
    ; ─────────────────────────────────────────────────────────────────────────
    mov si, msg_loading     ; SI = Pointer zur Nachricht
    call print_string       ; String ausgeben

    ; ─────────────────────────────────────────────────────────────────────────
    ; Schritt 4: Stage 2 von Disk laden
    ; ─────────────────────────────────────────────────────────────────────────
    call load_stage2

    ; ─────────────────────────────────────────────────────────────────────────
    ; Schritt 5: Erfolgsmeldung und zu Stage 2 springen
    ; ─────────────────────────────────────────────────────────────────────────
    mov si, msg_ok
    call print_string

    ; Far Jump zu Stage 2
    ; Wir springen zu STAGE2_LOAD_SEG:STAGE2_LOAD_OFF = 0x0000:0x7E00
    jmp STAGE2_LOAD_SEG:STAGE2_LOAD_OFF

; =============================================================================
; Funktion: print_string
; Gibt null-terminierten String auf Bildschirm aus
; Input: SI = Pointer zum String
; =============================================================================
print_string:
    pusha                   ; Alle Register auf Stack sichern
                            ; (AX, CX, DX, BX, SP, BP, SI, DI)
.loop:
    lodsb                   ; AL = [SI], dann SI++
                            ; LODSB = Load String Byte

    test al, al             ; Ist AL = 0?
                            ; TEST macht AND, setzt aber nur Flags

    jz .done                ; Wenn Zero Flag gesetzt → String Ende

    mov ah, 0x0E            ; BIOS Funktion: Teletype Output
    mov bh, 0               ; Page Number
    int 0x10                ; BIOS Video Interrupt aufrufen
                            ; Gibt Zeichen in AL aus

    jmp .loop               ; Nächstes Zeichen

.done:
    popa                    ; Alle Register wiederherstellen
    ret                     ; Zurück zum Aufrufer

; =============================================================================
; Funktion: load_stage2
; Lädt Stage 2 von Disk in den Speicher
; =============================================================================
load_stage2:
    pusha

    ; ─────────────────────────────────────────────────────────────────────────
    ; BIOS INT 0x13, AH=0x02: Sektoren lesen
    ; ─────────────────────────────────────────────────────────────────────────
    ; Parameter:
    ;   AH = 0x02 (Funktion: Read Sectors)
    ;   AL = Anzahl Sektoren
    ;   CH = Zylinder (untere 8 Bits)
    ;   CL = Sektor (Bits 0-5) + Zylinder obere 2 Bits (Bits 6-7)
    ;   DH = Head
    ;   DL = Drive Nummer
    ;   ES:BX = Zieladresse im Speicher
    ;
    ; Rückgabe:
    ;   CF = 0: Erfolg, AH = 0
    ;   CF = 1: Fehler, AH = Fehlercode

    mov ah, 0x02            ; Funktion: Read Sectors
    mov al, STAGE2_SECTORS  ; Anzahl Sektoren
    mov ch, 0               ; Zylinder 0
    mov cl, 2               ; Sektor 2 (Sektor 1 = unser Bootloader)
                            ; Sektoren sind 1-basiert!
    mov dh, 0               ; Head 0
    mov dl, [boot_drive]    ; Drive Nummer (vom BIOS)
    mov bx, STAGE2_LOAD_SEG ; Ziel-Segment
    mov es, bx              ; ES = Segment
    mov bx, STAGE2_LOAD_OFF ; BX = Offset
                            ; Zusammen: ES:BX = 0x0000:0x7E00

    int 0x13                ; BIOS Disk Interrupt aufrufen

    jc disk_error           ; Jump if Carry → Fehler aufgetreten

    ; Prüfen ob alle Sektoren gelesen wurden
    cmp al, STAGE2_SECTORS
    jne disk_error

    popa
    ret

; =============================================================================
; Fehlerbehandlung
; =============================================================================
disk_error:
    mov si, msg_disk_error
    call print_string
    
    ; Fehlercode ausgeben (in AH nach fehlgeschlagenem INT 13h)
    mov al, ah              ; Fehlercode nach AL
    call print_hex_byte     ; Als Hex ausgeben
    
.halt:
    cli                     ; Interrupts aus
    hlt                     ; CPU anhalten
    jmp .halt               ; Falls Interrupt aufweckt

; =============================================================================
; Funktion: print_hex_byte
; Gibt ein Byte als Hex aus (z.B. 0x3F → "3F")
; Input: AL = Byte
; =============================================================================
print_hex_byte:
    pusha
    
    mov cl, al              ; Originalbyte sichern
    
    ; Oberes Nibble (obere 4 Bits)
    shr al, 4               ; AL = AL >> 4
    call .print_nibble
    
    ; Unteres Nibble (untere 4 Bits)
    mov al, cl
    and al, 0x0F            ; Nur untere 4 Bits
    call .print_nibble
    
    popa
    ret

.print_nibble:
    ; Wandelt 0-15 in '0'-'9' oder 'A'-'F' um
    cmp al, 10
    jl .is_digit
    add al, 'A' - 10        ; 10 → 'A', 11 → 'B', ...
    jmp .output
.is_digit:
    add al, '0'             ; 0 → '0', 1 → '1', ...
.output:
    mov ah, 0x0E
    int 0x10
    ret

; =============================================================================
; Daten
; =============================================================================
msg_loading:    db "KiOS Stage1 - Loading Stage2...", 13, 10, 0
                ;                                     ^   ^
                ;                                     CR  LF (Zeilenumbruch)

msg_ok:         db "OK!", 13, 10, 0

msg_disk_error: db 13, 10, "Disk Error: 0x", 0

boot_drive:     db 0        ; Hier speichern wir die Drive-Nummer

; =============================================================================
; Padding und Boot-Signatur
; =============================================================================
; Bootsektor MUSS genau 512 Bytes sein und mit 0xAA55 enden

times 510 - ($ - $$) db 0   ; Mit Nullen auffüllen bis Byte 510
                            ; $ = aktuelle Position
                            ; $$ = Start der Sektion
                            ; $ - $$ = bisherige Größe

dw 0xAA55                   ; Magic Number (Little Endian: 0x55, 0xAA)
                            ; BIOS prüft diese Signatur!
