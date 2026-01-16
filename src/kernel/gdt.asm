; GDT für x86_64 mit TSS-Deskriptor
; Wird im BSS/Kernel als gdt[] angelegt und von C initialisiert

section .bss
align 16
global gdt

gdt:
    resq 6      ; Platz für 6 Einträge (Null, Code, Data, TSS Low, TSS High, evtl. Reserve)
