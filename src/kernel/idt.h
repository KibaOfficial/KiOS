// Copyright (c) 2026 KibaOfficial
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

/*
 * KiOS - Interrupt Descriptor Table (IDT)
 *
 * Die IDT ist eine Tabelle, die definiert, welche Handler-Funktionen
 * aufgerufen werden, wenn ein Interrupt oder eine Exception auftritt.
 *
 * x86_64 IDT Entry Format (16 Bytes):
 *   Offset 0-1:  Handler Address [0:15]
 *   Offset 2-3:  Code Segment Selector
 *   Offset 4:    IST (Interrupt Stack Table, 0 = nicht benutzt)
 *   Offset 5:    Type & Attributes
 *   Offset 6-7:  Handler Address [16:31]
 *   Offset 8-11: Handler Address [32:63]
 *   Offset 12-15: Reserved (0)
 */

#ifndef KIOS_IDT_H
#define KIOS_IDT_H

#include "types.h"

/* IDT Entry (Gate Descriptor) - 16 Bytes in 64-bit mode */
typedef struct {
    uint16_t offset_low;    /* Handler Address bits 0-15 */
    uint16_t selector;      /* Code Segment Selector (meist 0x08) */
    uint8_t  ist;           /* Interrupt Stack Table (0 = nicht benutzt) */
    uint8_t  type_attr;     /* Type und Attributes */
    uint16_t offset_mid;    /* Handler Address bits 16-31 */
    uint32_t offset_high;   /* Handler Address bits 32-63 */
    uint32_t reserved;      /* Reserved, muss 0 sein */
} __attribute__((packed)) idt_entry_t;

/* IDT Pointer - Zeiger auf die IDT */
typedef struct {
    uint16_t limit;         /* Größe der IDT - 1 */
    uint64_t base;          /* Adresse der IDT */
} __attribute__((packed)) idt_ptr_t;

/* Type/Attribute Flags */
#define IDT_TYPE_INTERRUPT  0x8E    /* 64-bit Interrupt Gate (P=1, DPL=0) */
#define IDT_TYPE_TRAP       0x8F    /* 64-bit Trap Gate (P=1, DPL=0) */
#define IDT_TYPE_USER_INT   0xEE    /* User-Mode Interrupt (DPL=3) */

/* Anzahl der IDT-Einträge */
#define IDT_ENTRIES 256

/* Exception-Nummern */
#define EXC_DIVIDE_ERROR        0   /* Division by Zero */
#define EXC_DEBUG               1   /* Debug Exception */
#define EXC_NMI                 2   /* Non-Maskable Interrupt */
#define EXC_BREAKPOINT          3   /* Breakpoint */
#define EXC_OVERFLOW            4   /* Overflow */
#define EXC_BOUND_RANGE         5   /* Bound Range Exceeded */
#define EXC_INVALID_OPCODE      6   /* Invalid Opcode */
#define EXC_DEVICE_NOT_AVAIL    7   /* Device Not Available */
#define EXC_DOUBLE_FAULT        8   /* Double Fault */
#define EXC_COPROCESSOR         9   /* Coprocessor Segment Overrun */
#define EXC_INVALID_TSS         10  /* Invalid TSS */
#define EXC_SEGMENT_NOT_PRESENT 11  /* Segment Not Present */
#define EXC_STACK_FAULT         12  /* Stack Fault */
#define EXC_GENERAL_PROTECTION  13  /* General Protection Fault */
#define EXC_PAGE_FAULT          14  /* Page Fault */
#define EXC_FPU_ERROR           16  /* x87 FPU Error */
#define EXC_ALIGNMENT_CHECK     17  /* Alignment Check */
#define EXC_MACHINE_CHECK       18  /* Machine Check */
#define EXC_SIMD_FP_ERROR       19  /* SIMD Floating-Point Exception */

/* IRQ-Nummern (nach Remapping auf 32-47) */
#define IRQ0  32    /* Timer */
#define IRQ1  33    /* Keyboard */
#define IRQ2  34    /* Cascade (unused) */
#define IRQ3  35    /* COM2 */
#define IRQ4  36    /* COM1 */
#define IRQ5  37    /* LPT2 */
#define IRQ6  38    /* Floppy */
#define IRQ7  39    /* LPT1 */
#define IRQ8  40    /* RTC */
#define IRQ9  41    /* Free */
#define IRQ10 42    /* Free */
#define IRQ11 43    /* Free */
#define IRQ12 44    /* PS/2 Mouse */
#define IRQ13 45    /* FPU */
#define IRQ14 46    /* Primary ATA */
#define IRQ15 47    /* Secondary ATA */

/* Funktionen */
void idt_init(void);
void idt_set_gate(uint8_t num, uint64_t handler, uint16_t selector, uint8_t flags);
void idt_set_gate_ist(uint8_t num, uint64_t handler, uint16_t selector, uint8_t flags, uint8_t ist);

/* Externe ISR-Deklarationen (definiert in idt_asm.asm) */
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

/* IRQ-Handler (32-47) */
extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

#endif /* KIOS_IDT_H */
