// Copyright (c) 2026 KibaOfficial
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

/*
 * KiOS - Interrupt Service Routines (ISR)
 *
 * Handler-Funktionen für Exceptions und IRQs
 */

#ifndef KIOS_ISR_H
#define KIOS_ISR_H

#include "types.h"

/* Register-Frame (wird vom ISR-Stub auf den Stack gepusht) */
typedef struct {
    /* Segment-Register */
    uint64_t gs, fs, es, ds;

    /* General Purpose Register (gepusht von isr_common_stub) */
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;

    /* Interrupt-Nummer und Error Code */
    uint64_t int_no, err_code;

    /* Von CPU automatisch gepusht */
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed)) registers_t;

/* IRQ-Handler Typ */
typedef void (*irq_handler_t)(registers_t*);

/* Funktionen */
void isr_handler(registers_t* regs);
void irq_handler(registers_t* regs);
void irq_install_handler(int irq, irq_handler_t handler);
void irq_uninstall_handler(int irq);

#endif /* KIOS_ISR_H */
