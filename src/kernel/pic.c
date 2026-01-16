// Copyright (c) 2026 KibaOfficial
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "pic.h"
#include "io.h"

/*
 * pic_remap - Remappt die PIC IRQs auf neue Interrupt-Nummern
 *
 * offset1: Interrupt-Offset für Master PIC (normalerweise 32)
 * offset2: Interrupt-Offset für Slave PIC (normalerweise 40)
 *
 * Nach dem Remapping:
 *   IRQ 0-7  -> INT 32-39 (Master PIC)
 *   IRQ 8-15 -> INT 40-47 (Slave PIC)
 */
void pic_remap(int offset1, int offset2) {
    uint8_t a1, a2;

    /* Aktuelle Masken sichern */
    a1 = inb(PIC1_DATA);
    a2 = inb(PIC2_DATA);

    /* Initialisierungs-Sequenz starten (ICW1) */
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    /* ICW2: Vector Offsets */
    outb(PIC1_DATA, offset1);    /* Master PIC Offset */
    io_wait();
    outb(PIC2_DATA, offset2);    /* Slave PIC Offset */
    io_wait();

    /* ICW3: Cascade-Konfiguration */
    outb(PIC1_DATA, 0x04);       /* Master: Slave ist an IRQ2 */
    io_wait();
    outb(PIC2_DATA, 0x02);       /* Slave: Cascade Identity = 2 */
    io_wait();

    /* ICW4: 8086 Mode */
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    /* Masken wiederherstellen */
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}

/*
 * pic_disable - Deaktiviert beide PICs
 */
void pic_disable(void) {
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

/*
 * pic_send_eoi - Sendet End-of-Interrupt Signal
 */
void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}

/*
 * pic_set_mask - Maskiert (deaktiviert) einen IRQ
 */
void pic_set_mask(uint8_t irq_line) {
    uint16_t port;
    uint8_t value;

    if (irq_line < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq_line -= 8;
    }
    value = inb(port) | (1 << irq_line);
    outb(port, value);
}

/*
 * pic_clear_mask - Demaskiert (aktiviert) einen IRQ
 */
void pic_clear_mask(uint8_t irq_line) {
    uint16_t port;
    uint8_t value;

    if (irq_line < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq_line -= 8;
    }
    value = inb(port) & ~(1 << irq_line);
    outb(port, value);
}
