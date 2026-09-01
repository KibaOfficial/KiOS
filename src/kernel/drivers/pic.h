// Copyright (c) 2026 KibaOfficial
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

/*
 * KiOS - PIC (Programmable Interrupt Controller)
 *
 * Der 8259 PIC verwaltet Hardware-Interrupts (IRQs).
 * Im PC gibt es zwei PICs (Master und Slave), die zusammen 16 IRQs verwalten.
 *
 * Problem: Die PICs sind standardmäßig so konfiguriert, dass sie IRQs
 * auf Interrupt 0-15 mappen, was mit CPU-Exceptions kollidiert!
 *
 * Lösung: IRQ-Remapping - Wir mappen IRQs auf Interrupt 32-47.
 */

#ifndef KIOS_PIC_H
#define KIOS_PIC_H

#include "types.h"

/* PIC Ports */
#define PIC1_COMMAND    0x20
#define PIC1_DATA       0x21
#define PIC2_COMMAND    0xA0
#define PIC2_DATA       0xA1

/* PIC Commands */
#define PIC_EOI         0x20    /* End of Interrupt */

#define ICW1_ICW4       0x01    /* ICW4 needed */
#define ICW1_SINGLE     0x02    /* Single (cascade) mode */
#define ICW1_INTERVAL4  0x04    /* Call address interval 4 (8) */
#define ICW1_LEVEL      0x08    /* Level triggered (edge) mode */
#define ICW1_INIT       0x10    /* Initialization */

#define ICW4_8086       0x01    /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO       0x02    /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE  0x08    /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C    /* Buffered mode/master */
#define ICW4_SFNM       0x10    /* Special fully nested (not) */

/* Funktionen */
void pic_remap(int offset1, int offset2);
void pic_disable(void);
void pic_send_eoi(uint8_t irq);
void pic_set_mask(uint8_t irq_line);
void pic_clear_mask(uint8_t irq_line);

#endif /* KIOS_PIC_H */
