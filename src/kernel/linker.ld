/*
 * KiOS Linker Script
 *
 * Definiert das Speicherlayout des Kernels.
 * Der Kernel wird an 0x100000 (1MB) geladen.
 */

/* Entry Point - wo der Kernel startet */
ENTRY(_start)

/* Basisadresse */
KERNEL_BASE = 0x100000;

SECTIONS
{
    /* Kernel startet bei 1MB */
    . = KERNEL_BASE;
    
    /* Text Section - Ausführbarer Code */
    .text : ALIGN(4096)
    {
        *(.text)
        *(.text.*)
    }
    
    /* Read-Only Data */
    .rodata : ALIGN(4096)
    {
        *(.rodata)
        *(.rodata.*)
    }
    
    /* Initialized Data */
    .data : ALIGN(4096)
    {
        *(.data)
        *(.data.*)
    }
    
    /* Uninitialized Data (BSS) */
    .bss : ALIGN(4096)
    {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        __bss_end = .;
    }
    
    /* Kernel Ende */
    __kernel_end = .;
    
    /* Debug Sections verwerfen */
    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
        *(.note.*)
    }
}
