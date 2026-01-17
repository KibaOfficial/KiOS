/**
 * Copyright (c) 2026 KibaOfficial
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
#ifndef KIOS_PIT_H
#define KIOS_PIT_H

#include "types.h"

/* =============================================================================
 * PIT (Programmable Interval Timer) - 8253/8254 Chip
 * =============================================================================
 * Der PIT ist unser "Metronom" - er tickt in regelmäßigen Abständen und
 * löst IRQ0 aus. Das ist die Grundlage für:
 *   - Uptime tracking
 *   - sleep() Funktionen
 *   - Preemptive Multitasking (Task-Switching alle X ms)
 *
 * Hardware Details:
 *   - Base Frequency: 1.193182 MHz (1193182 Hz)
 *   - Channel 0: System Timer (IRQ0)
 *   - Channel 1: DRAM Refresh (nicht mehr genutzt)
 *   - Channel 2: PC Speaker
 *
 * I/O Ports:
 *   0x40: Channel 0 Data Port
 *   0x41: Channel 1 Data Port
 *   0x42: Channel 2 Data Port
 *   0x43: Mode/Command Register
 */

// PIT I/O Ports
#define PIT_CHANNEL0_DATA   0x40    // Channel 0 Data Port (System Timer)
#define PIT_CHANNEL1_DATA   0x41    // Channel 1 Data Port (unused)
#define PIT_CHANNEL2_DATA   0x42    // Channel 2 Data Port (PC Speaker)
#define PIT_COMMAND         0x43    // Mode/Command Register

// PIT Base Frequency (1.193182 MHz)
#define PIT_BASE_FREQ       1193182

// Target Frequency (100 Hz = 100 Ticks pro Sekunde)
#define PIT_TARGET_FREQ     100

// Divisor = Base Frequency / Target Frequency
#define PIT_DIVISOR         (PIT_BASE_FREQ / PIT_TARGET_FREQ)

/* =============================================================================
 * Funktionen
 * =============================================================================
 */

/**
 * pit_init - Initialisiert den PIT und startet den Timer
 *
 * Konfiguriert Channel 0 auf 100Hz (alle 10ms ein Tick).
 * Registriert den IRQ0 Handler für Timer-Interrupts.
 */
void pit_init(void);

/**
 * pit_get_ticks - Gibt die Anzahl der Timer-Ticks seit Systemstart zurück
 *
 * @return Anzahl der Ticks (bei 100Hz = Ticks / 100 = Sekunden)
 */
uint64_t pit_get_ticks(void);

/**
 * pit_get_uptime_seconds - Gibt die Uptime in Sekunden zurück
 *
 * @return Sekunden seit Systemstart
 */
uint64_t pit_get_uptime_seconds(void);

/**
 * pit_enable_scheduler - Aktiviert den Task-Scheduler im Timer-Interrupt
 *
 * Der Scheduler wechselt alle 100ms (10 Ticks) zwischen Tasks.
 * Sollte erst aufgerufen werden, nachdem Tasks erstellt wurden.
 */
void pit_enable_scheduler(void);

#endif /* KIOS_PIT_H */
