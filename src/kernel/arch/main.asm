org 0x0 ; Stage2 loads us at 0x2000:0x0000
bits 16

%define ENDL 0x0D, 0x0A

start:
	; Setup segments
	mov ax, 0x2000
	mov ds, ax
	mov es, ax

	; Setup stack (below kernel at 0x1000:0xFFFF)
	mov ax, 0x1000
	mov ss, ax
	mov sp, 0xFFF0

	; Clear screen first
	call clear_screen

	; Print kernel message
	mov si, msg_hello
	call print_string

	; Print additional info
	mov si, msg_kernel_info
	call print_string

.halt:
	cli
	hlt
	jmp .halt	; In case of NMI

;
; Clears the screen using BIOS interrupt
;
clear_screen:
	push ax
	push bx
	push cx
	push dx

	mov ah, 0x00	; Set video mode
	mov al, 0x03	; 80x25 text mode
	int 0x10

	; Set cursor position to 0,0
	mov ah, 0x02
	mov bh, 0x00
	xor dx, dx
	int 0x10

	pop dx
	pop cx
	pop bx
	pop ax
	ret

;
; Prints a null-terminated string pointed to by SI to the screen
; Params:
;	- ds:si -> pointer to null-terminated string
;
print_string:
	push si
	push ax
	push bx

.loop:
	lodsb
	or al, al
	jz .done

	mov ah, 0x0E
	mov bh, 0x00
	int 0x10

	jmp .loop

.done:
	pop bx
	pop ax
	pop si
	ret

;
; Data section
;
msg_hello:       db "=================================", ENDL
                 db " Welcome to KiOS Kernel!", ENDL
                 db "=================================", ENDL, ENDL, 0

msg_kernel_info: db "Kernel loaded successfully at 0x2000:0x0000", ENDL
                 db "CPU Mode: Real Mode (16-bit)", ENDL
                 db "Stack: 0x1000:0xFFF0", ENDL, ENDL
                 db "System halted. Press reset to reboot.", ENDL, 0