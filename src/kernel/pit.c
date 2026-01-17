/**
 * Copyright (c) 2026 KibaOfficial
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
#include "pit.h"
#include "io.h"
#include "isr.h"
#include "task.h"

/* =============================================================================
 * Globale Variablen
 * =============================================================================
 */

// Timer Tick Counter (wird bei jedem IRQ0 erhöht)
static volatile uint64_t pit_ticks = 0;

// Scheduler aktiviert?
static bool scheduler_enabled = false;

/* =============================================================================
 * IRQ0 Handler - Timer Interrupt
 * =============================================================================
 */

/**
 * pit_irq_handler - Wird bei jedem Timer-Tick aufgerufen (alle 10ms bei 100Hz)
 *
 * @param regs Register Frame vom Interrupt
 */
static void pit_irq_handler(registers_t *regs) {
    pit_ticks++;

    // Scheduler alle 10 Ticks aufrufen (= alle 100ms bei 100Hz)
    if (scheduler_enabled && (pit_ticks % 10 == 0)) {
        // Task-Switch durchführen
        registers_t *new_regs = task_switch(regs);

        // Falls ein anderer Task gewählt wurde, Stack-Pointer umbiegen
        if (new_regs != regs) {
            irq_set_new_stack(new_regs);
        }
    }
}

/* =============================================================================
 * Public Funktionen
 * =============================================================================
 */

/**
 * pit_init - Initialisiert den PIT
 *
 * Konfiguriert Channel 0 im Mode 3 (Square Wave Generator) auf 100Hz.
 * Registriert den IRQ0 Handler.
 *
 * Command Byte Format (Port 0x43):
 *   Bits 7-6: Channel Select (00 = Channel 0)
 *   Bits 5-4: Access Mode (11 = Low byte / High byte)
 *   Bits 3-1: Operating Mode (011 = Mode 3: Square Wave)
 *   Bit 0:    BCD/Binary (0 = Binary)
 *
 *   Binary: 00110110 = 0x36
 */
void pit_init(void) {
    // IRQ0 Handler registrieren
    irq_install_handler(0, pit_irq_handler);  // IRQ0

    // Command Byte senden: Channel 0, Access Mode Lobyte/Hibyte, Mode 3, Binary
    outb(PIT_COMMAND, 0x36);

    // Divisor senden (Low byte dann High byte)
    uint16_t divisor = PIT_DIVISOR;
    outb(PIT_CHANNEL0_DATA, (uint8_t)(divisor & 0xFF));        // Low byte
    outb(PIT_CHANNEL0_DATA, (uint8_t)((divisor >> 8) & 0xFF)); // High byte

    // IRQ0 unmaskieren (PIT ist jetzt aktiv!)
    // pic_clear_mask(0);  // Wird in main.c gemacht
}

/**
 * pit_get_ticks - Gibt die Anzahl der Timer-Ticks zurück
 *
 * @return Tick-Counter
 */
uint64_t pit_get_ticks(void) {
    return pit_ticks;
}

/**
 * pit_get_uptime_seconds - Gibt die Uptime in Sekunden zurück
 *
 * @return Sekunden seit Systemstart
 */
uint64_t pit_get_uptime_seconds(void) {
    return pit_ticks / PIT_TARGET_FREQ;  // Bei 100Hz: Ticks / 100 = Sekunden
}

/**
 * pit_enable_scheduler - Aktiviert den Scheduler
 */
void pit_enable_scheduler(void) {
    scheduler_enabled = true;
}
