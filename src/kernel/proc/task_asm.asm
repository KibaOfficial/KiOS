; Copyright (c) 2026 KibaOfficial
; Task Switch Helper - direkt in einen Task springen via iretq
; MIT License

[bits 64]
section .text

; =============================================================================
; task_restore(registers_t *regs)
;
; Springt direkt in einen Task via iretq.
; Wird von task_exit_current() aufgerufen um sofort zur Shell zu wechseln.
;
; Parameter: rdi = Pointer auf registers_t des Zieltasks
;
; registers_t Layout (aus isr.h):
;   +0x00  r15
;   +0x08  r14
;   +0x10  r13
;   +0x18  r12
;   +0x20  r11
;   +0x28  r10
;   +0x30  r9
;   +0x38  r8
;   +0x40  rbp
;   +0x48  rdi
;   +0x50  rsi
;   +0x58  rdx
;   +0x60  rcx
;   +0x68  rbx
;   +0x70  rax
;   +0x78  int_no
;   +0x80  err_code
;   +0x88  rip      <-- iretq liest ab hier
;   +0x90  cs
;   +0x98  rflags
;   +0xA0  rsp
;   +0xA8  ss
; =============================================================================

global task_restore
task_restore:
    ; rdi = &regs
 
    ; Wir brauchen iretq Stack Frame:
    ; [ss, rsp, rflags, cs, rip] von oben nach unten auf dem Stack
    ;
    ; Trick: Wir setzen RSP direkt auf regs->rip (Offset 0x88)
    ; dann liegen rip/cs/rflags/rsp/ss genau da wo iretq sie erwartet!
 
    ; Alle GP-Register laden (rdi brauchen wir als Pointer, laden wir zuletzt)
	mov rax, [rdi + 0x090] ; rflags
    mov rbx, [rdi + 0x088] ; rip
    mov rcx, [rdi + 0x080] ; err_code
    mov rdx, [rdi + 0x078] ; int_no
    mov rsi, [rdi + 0x070] ; rax
    mov rbp, [rdi + 0x060] ; rbx
    mov r8,  [rdi + 0x058] ; rcx
    mov r9,  [rdi + 0x050] ; rdx
    mov r10, [rdi + 0x048] ; rsi
    mov r11, [rdi + 0x040] ; rdi
    mov r12, [rdi + 0x038] ; rbp
    mov r13, [rdi + 0x030] ; r8
    mov r14, [rdi + 0x028] ; r9
    mov r15, [rdi + 0x020] ; r10

    ; RSP auf den iretq-Frame zeigen lassen:
    ; Bei Offset 0x0A8 liegt rip - genau da wo iretq anfängt zu lesen.
    ; iretq erwartet auf dem Stack: rip, cs, rflags, rsp, ss
    lea rsp, [rdi + 0x0A8] ; rsp = &regs->rip

    ; rid jetzt erst laden (haben es als pointer gebraucht)
    mov rdi, [rdi + 0x068] ; r11 = rdi (originaler Wert vor Interrupt)

    ; interrupts aus - iretq stellt sie aus RFLAGS wieder her
    cli

    ; Sicherstellen dass IF gesetzt ist im RFLAGS auf dem Stack
    ; RFLAGS liegt 8 Bytes nach rip, also bei [rsp+8+8] = [rsp+16]
    or qword [rsp + 16], 0x200   ; IF-Bit setzen

    ; sprung! iretq liest rip/cs/rflags/rsp/ss und springt rein
    iretq