// Copyright (c) 2026 KibaOfficial
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "isr.h"
#include "idt.h"
#include "vga.h"
#include "io.h"

/* PIC (Programmable Interrupt Controller) Ports */
#define PIC1_COMMAND    0x20
#define PIC1_DATA       0x21
#define PIC2_COMMAND    0xA0
#define PIC2_DATA       0xA1

/* PIC EOI (End of Interrupt) */
#define PIC_EOI         0x20

/* Exception-Namen */
static const char* exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 FPU Error",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Security Exception",
    "Reserved"
};

/* IRQ-Handler Array */
static irq_handler_t irq_handlers[16] = {0};

/* Globaler Pointer für Task-Switching */
static registers_t *new_task_regs = NULL;

/*
 * isr_handler - Gemeinsamer Handler für alle Exceptions
 */
void isr_handler(registers_t* regs) {
    vga_set_color(VGA_WHITE, VGA_RED);
    vga_println("");
    vga_println("===========================================");
    vga_print("  EXCEPTION: ");

    if (regs->int_no < 32) {
        vga_println(exception_messages[regs->int_no]);
    } else {
        vga_print("Unknown Exception #");
        vga_print_dec(regs->int_no);
        vga_println("");
    }

    vga_println("===========================================");
    vga_set_color(VGA_LIGHT_GRAY, VGA_BLACK);

    vga_print("  INT#:    ");
    vga_print_dec(regs->int_no);
    vga_println("");

    vga_print("  ERRCODE: 0x");
    vga_print_hex(regs->err_code);
    vga_println("");

    vga_print("  RIP:     0x");
    vga_print_hex(regs->rip);
    vga_println("");

    vga_print("  CS:      0x");
    vga_print_hex(regs->cs);
    vga_println("");

    vga_print("  RFLAGS:  0x");
    vga_print_hex(regs->rflags);
    vga_println("");

    vga_print("  RSP:     0x");
    vga_print_hex(regs->rsp);
    vga_println("");

    vga_print("  SS:      0x");
    vga_print_hex(regs->ss);
    vga_println("");

    vga_println("");
    vga_print("  RAX: 0x");
    vga_print_hex(regs->rax);
    vga_print("  RBX: 0x");
    vga_print_hex(regs->rbx);
    vga_println("");

    vga_print("  RCX: 0x");
    vga_print_hex(regs->rcx);
    vga_print("  RDX: 0x");
    vga_print_hex(regs->rdx);
    vga_println("");

    vga_println("===========================================");
    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_println("");
    vga_println("System halted.");

    /* System anhalten */
    __asm__ volatile("cli");
    for (;;) {
        __asm__ volatile("hlt");
    }
}

/*
 * irq_handler - Gemeinsamer Handler für alle IRQs
 *
 * Gibt den (möglicherweise neuen) Stack-Pointer zurück.
 * Der Scheduler kann den Stack-Pointer ändern für Task-Switching.
 */
registers_t* irq_handler(registers_t* regs) {
    /* IRQ-Nummer berechnen (32-47 -> 0-15) */
    int irq = regs->int_no - 32;

    /* Globale Variable für Task-Switching zurücksetzen */
    new_task_regs = NULL;

    /* Custom Handler aufrufen, falls registriert */
    if (irq_handlers[irq] != 0) {
        irq_handler_t handler = irq_handlers[irq];
        handler(regs);
    }

    /* EOI (End of Interrupt) an PIC senden */
    if (irq >= 8) {
        /* Slave PIC (IRQ 8-15) */
        outb(PIC2_COMMAND, PIC_EOI);
    }
    /* Master PIC (IRQ 0-7) */
    outb(PIC1_COMMAND, PIC_EOI);

    /* Stack-Pointer zurückgeben */
    /* Falls der Handler new_task_regs gesetzt hat, nutzen wir den neuen Stack */
    return (new_task_regs != NULL) ? new_task_regs : regs;
}

/*
 * irq_set_new_stack - Vom IRQ-Handler aufrufen um Stack zu wechseln
 */
void irq_set_new_stack(registers_t *new_regs) {
    new_task_regs = new_regs;
}

/*
 * irq_install_handler - Registriert einen IRQ-Handler
 */
void irq_install_handler(int irq, irq_handler_t handler) {
    if (irq >= 0 && irq < 16) {
        irq_handlers[irq] = handler;
    }
}

/*
 * irq_uninstall_handler - Entfernt einen IRQ-Handler
 */
void irq_uninstall_handler(int irq) {
    if (irq >= 0 && irq < 16) {
        irq_handlers[irq] = 0;
    }
}
