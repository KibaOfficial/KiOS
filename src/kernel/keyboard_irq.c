// Copyright (c) 2026 KibaOfficial
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

/*
 * KiOS - Keyboard Interrupt Handler
 *
 * Interrupt-basierte Tastatur-Eingabe (IRQ1)
 * Ersetzt das alte Polling-System mit einem effizienten Interrupt-Handler
 */

#include "keyboard.h"
#include "io.h"
#include "isr.h"
#include "pic.h"

/* Keyboard Ports */
#define KB_DATA_PORT    0x60
#define KB_STATUS_PORT  0x64

/* Keyboard Buffer - Ring Buffer für eingehende Scancodes */
#define KB_BUFFER_SIZE 256
static uint8_t kb_buffer[KB_BUFFER_SIZE];
static volatile int kb_buffer_read = 0;
static volatile int kb_buffer_write = 0;

/*
 * keyboard_irq_handler - IRQ1 Handler (wird bei jedem Tastendruck aufgerufen)
 */
static void keyboard_irq_handler(registers_t* regs) {
    (void)regs; /* Nicht benutzt */

    /* Scancode vom Keyboard-Controller lesen */
    uint8_t scancode = inb(KB_DATA_PORT);

    /* In Ring-Buffer schreiben */
    int next_write = (kb_buffer_write + 1) % KB_BUFFER_SIZE;

    /* Nur schreiben wenn Buffer nicht voll */
    if (next_write != kb_buffer_read) {
        kb_buffer[kb_buffer_write] = scancode;
        kb_buffer_write = next_write;
    }
    /* Falls Buffer voll: Scancode verwerfen (könnte man auch anders lösen) */
}

/*
 * kb_irq_has_scancode - Prüft ob Scancodes im Buffer sind
 */
int kb_irq_has_scancode(void) {
    return kb_buffer_read != kb_buffer_write;
}

/*
 * kb_irq_get_scancode - Holt nächsten Scancode aus dem Buffer
 * Gibt 0 zurück wenn Buffer leer
 */
uint8_t kb_irq_get_scancode(void) {
    if (kb_buffer_read == kb_buffer_write) {
        return 0; /* Buffer leer */
    }

    uint8_t scancode = kb_buffer[kb_buffer_read];
    kb_buffer_read = (kb_buffer_read + 1) % KB_BUFFER_SIZE;
    return scancode;
}

/*
 * keyboard_irq_init - Initialisiert Keyboard Interrupt Handler
 */
void keyboard_irq_init(void) {
    /* Buffer initialisieren */
    kb_buffer_read = 0;
    kb_buffer_write = 0;

    /* IRQ1 Handler registrieren */
    irq_install_handler(1, keyboard_irq_handler);

    /* IRQ1 (Keyboard) aktivieren */
    pic_clear_mask(1);
}

/*
 * kb_getchar_irq - Liest ein Zeichen (blockierend, IRQ-basiert)
 */
char kb_getchar_irq(void) {
    while (1) {
        /* Warten bis Scancode im IRQ-Buffer verfügbar */
        while (!kb_irq_has_scancode()) {
            __asm__ volatile("hlt"); /* CPU schlafen legen bis nächster Interrupt */
        }

        uint8_t scancode = kb_irq_get_scancode();

        /* Scancode zu ASCII konvertieren (aus keyboard.h) */
        extern char kb_scancode_to_ascii(uint8_t);
        char c = kb_scancode_to_ascii(scancode);

        if (c != 0) {
            return c;
        }
    }
}

/*
 * kb_try_getchar_irq - Versucht ein Zeichen zu lesen (nicht blockierend, IRQ-basiert)
 */
char kb_try_getchar_irq(void) {
    if (!kb_irq_has_scancode()) {
        return 0;
    }

    uint8_t scancode = kb_irq_get_scancode();

    /* Scancode zu ASCII konvertieren (aus keyboard.h) */
    extern char kb_scancode_to_ascii(uint8_t);
    return kb_scancode_to_ascii(scancode);
}
