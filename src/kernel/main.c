/**
 * Copyright (c) 2026 KibaOfficial
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
/*
 * KiOS - Kernel Main
 *
 * Dies ist der Haupteinstiegspunkt des Kernels.
 * Wird von entry.asm aufgerufen nachdem 64-Bit Mode aktiv ist.
 */

#include "types.h"
#include "vga.h"
#include "keyboard.h"
#include "keyboard_irq.h"
#include "shell.h"
#include "idt.h"
#include "tss.h"
#include "gdt.h"
#include "pic.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "mm/heap.h"



/*
 * kernel_main - Kernel Einstiegspunkt
 *
 * Diese Funktion wird von entry.asm aufgerufen.
 * Hier initialisieren wir alles und starten die Shell.
 */
void kernel_main(void)
{
    /* VGA initialisieren und Bildschirm löschen */
    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_clear();

    /* PIC konfigurieren (IRQs auf Interrupt 32-47 remappen) */
    pic_remap(32, 40);

    /* WICHTIG: Alle IRQs maskieren BEVOR wir die IDT laden! */
    /* Sonst kommen Timer-Interrupts rein bevor Handler bereit sind */
    for (int i = 0; i < 16; i++) {
        pic_set_mask(i);
    }


    /* Double-Fault IST-Stack reservieren (z.B. 8 KB, statisch im BSS) */
    static uint8_t df_stack[8192] __attribute__((aligned(16)));

    /* TSS initialisieren (IST1 = Double Fault Stack) */
    tss_init(df_stack, sizeof(df_stack));

    /* GDT initialisieren und TSS eintragen */
    gdt_init(&tss, sizeof(tss));

    /* IDT initialisieren (setzt isr8 mit ist=1) */
    idt_init();

    /* PMM Initialisieren */
    pmm_init();

    /* VMM Initialisieren */
    vmm_init();

    /* Heap Initialisieren */
    heap_init();

    /* Keyboard Interrupt Handler initialisieren (aktiviert IRQ1) */
    keyboard_irq_init();

    /* Interrupts aktivieren */
    __asm__ volatile("sti");

    /* ASCII-Banner ausgeben (nur Standard-ASCII, VGA-kompatibel) */
    vga_println("");
    vga_print_colored("    _  ___  ___  ____   ", VGA_LIGHT_CYAN, VGA_BLACK);
    vga_println("");
    vga_print_colored("   | |/ (_)/ _ \\/ ___|  ", VGA_LIGHT_CYAN, VGA_BLACK);
    vga_println("");
    vga_print_colored("   | ' /| | | | \\___ \\  ", VGA_LIGHT_CYAN, VGA_BLACK);
    vga_println("");
    vga_print_colored("   | . \\| | |_| |___) | ", VGA_LIGHT_CYAN, VGA_BLACK);
    vga_println("");
    vga_print_colored("   |_|\\_\\_|\\___/|____/  ", VGA_LIGHT_CYAN, VGA_BLACK);
    vga_println("");
    vga_println("");

    vga_print_colored("  Welcome to KiOS v0.3.0", VGA_YELLOW, VGA_BLACK);
    vga_println(" - A simple 64-bit operating system");
    vga_println("");

    vga_print("  Running in ");
    vga_print_colored("x86_64 Long Mode", VGA_LIGHT_GREEN, VGA_BLACK);
    vga_println("");

    vga_print("  Type ");
    vga_print_colored("help", VGA_LIGHT_CYAN, VGA_BLACK);
    vga_println(" for available commands.");
    vga_println("");

    /* Shell starten */
    shell_run();

    /* Sollten wir nie erreichen */
    for (;;)
    {
        __asm__ volatile("hlt");
    }
}
