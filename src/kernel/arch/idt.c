// Copyright (c) 2026 KibaOfficial
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "idt.h"
#include "vga.h"
#include "io.h"
#include "string.h"

/* IDT Table - 256 Einträge */
static idt_entry_t idt[IDT_ENTRIES];

/* IDT Pointer */
static idt_ptr_t idtp;

/* Externe Funktion zum Laden der IDT (definiert in idt_asm.asm) */
extern void idt_load(uint64_t);

/*
 * idt_set_gate - Setzt einen IDT-Eintrag
 */

void idt_set_gate(uint8_t num, uint64_t handler, uint16_t selector, uint8_t flags) {
    idt_set_gate_ist(num, handler, selector, flags, 0);
}

void idt_set_gate_ist(uint8_t num, uint64_t handler, uint16_t selector, uint8_t flags, uint8_t ist) {
    idt[num].offset_low  = (uint16_t)(handler & 0xFFFF);
    idt[num].offset_mid  = (uint16_t)((handler >> 16) & 0xFFFF);
    idt[num].offset_high = (uint32_t)((handler >> 32) & 0xFFFFFFFF);
    idt[num].selector = selector;
    idt[num].ist = ist; /* 0 = kein IST, 1 = IST1 (DF) */
    idt[num].type_attr = flags;
    idt[num].reserved = 0;
}

/*
 * idt_init - Initialisiert die IDT
 */
void idt_init(void) {
    /* IDT-Pointer setzen */
    idtp.limit = (sizeof(idt_entry_t) * IDT_ENTRIES) - 1;
    idtp.base = (uint64_t)&idt;

    /* IDT mit Nullen initialisieren */
    memset(&idt, 0, sizeof(idt_entry_t) * IDT_ENTRIES);


    /* Exception-Handler (0-31) registrieren, isr8 (Double Fault) mit IST1 */
    idt_set_gate(0, (uint64_t)isr0, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(1, (uint64_t)isr1, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(2, (uint64_t)isr2, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(3, (uint64_t)isr3, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(4, (uint64_t)isr4, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(5, (uint64_t)isr5, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(6, (uint64_t)isr6, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(7, (uint64_t)isr7, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate_ist(8, (uint64_t)isr8, 0x08, IDT_TYPE_INTERRUPT, 1); /* Double Fault mit IST1 */
    idt_set_gate(9, (uint64_t)isr9, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(10, (uint64_t)isr10, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(11, (uint64_t)isr11, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(12, (uint64_t)isr12, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(13, (uint64_t)isr13, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(14, (uint64_t)isr14, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(15, (uint64_t)isr15, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(16, (uint64_t)isr16, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(17, (uint64_t)isr17, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(18, (uint64_t)isr18, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(19, (uint64_t)isr19, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(20, (uint64_t)isr20, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(21, (uint64_t)isr21, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(22, (uint64_t)isr22, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(23, (uint64_t)isr23, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(24, (uint64_t)isr24, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(25, (uint64_t)isr25, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(26, (uint64_t)isr26, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(27, (uint64_t)isr27, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(28, (uint64_t)isr28, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(29, (uint64_t)isr29, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(30, (uint64_t)isr30, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(31, (uint64_t)isr31, 0x08, IDT_TYPE_INTERRUPT);

    /* IRQ-Handler (32-47) registrieren */
    idt_set_gate(32, (uint64_t)irq0, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(33, (uint64_t)irq1, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(34, (uint64_t)irq2, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(35, (uint64_t)irq3, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(36, (uint64_t)irq4, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(37, (uint64_t)irq5, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(38, (uint64_t)irq6, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(39, (uint64_t)irq7, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(40, (uint64_t)irq8, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(41, (uint64_t)irq9, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(42, (uint64_t)irq10, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(43, (uint64_t)irq11, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(44, (uint64_t)irq12, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(45, (uint64_t)irq13, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(46, (uint64_t)irq14, 0x08, IDT_TYPE_INTERRUPT);
    idt_set_gate(47, (uint64_t)irq15, 0x08, IDT_TYPE_INTERRUPT);

    /* IDT laden */
    idt_load((uint64_t)&idtp);
}
