/**
 * Copyright (c) 2026 KibaOfficial
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
#ifndef KIOS_TASK_H
#define KIOS_TASK_H

#include "types.h"
#include "isr.h"

/* =============================================================================
 * Task States
 * =============================================================================
 */

typedef enum {
    TASK_STATE_READY,      // Task ist bereit zum Ausführen
    TASK_STATE_RUNNING,    // Task wird gerade ausgeführt
    TASK_STATE_BLOCKED,    // Task wartet auf Event (z.B. I/O)
    TASK_STATE_SLEEPING,   // Task schläft (sleep())
    TASK_STATE_ZOMBIE      // Task ist beendet, wartet auf Cleanup
} task_state_t;

/* =============================================================================
 * Task Control Block (TCB)
 * =============================================================================
 */

#define TASK_NAME_MAX 32
#define MAX_TASKS 64

/**
 * task_t - Task Control Block
 *
 * Beschreibt einen einzelnen Task/Thread im System.
 */
typedef struct task {
    uint32_t pid;                    // Process ID
    char name[TASK_NAME_MAX];        // Task Name
    task_state_t state;              // Aktueller Zustand

    registers_t *regs;               // Gespeicherter CPU-Zustand (zeigt auf Stack)

    uint64_t stack_base;             // Basis-Adresse des Stacks
    uint64_t stack_size;             // Größe des Stacks in Bytes

    uint64_t sleep_until;            // Tick-Count bis Task aufwacht (bei SLEEPING)

    struct task *next;               // Nächster Task in der Queue (für Round-Robin)
} task_t;

/* =============================================================================
 * Public Functions
 * =============================================================================
 */

/**
 * task_init - Initialisiert das Task-Subsystem
 */
void task_init(void);

/**
 * task_create - Erstellt einen neuen Task
 *
 * @param name Task-Name
 * @param entry Einstiegspunkt (Funktionspointer)
 * @param stack_size Stack-Größe in Bytes
 * @return Pointer zum neuen Task oder NULL bei Fehler
 */
task_t* task_create(const char *name, void (*entry)(void), uint64_t stack_size);

/**
 * task_get_current - Gibt den aktuell laufenden Task zurück
 *
 * @return Pointer zum aktuellen Task
 */
task_t* task_get_current(void);

/**
 * task_switch - Wechselt zum nächsten Task (wird vom Scheduler aufgerufen)
 *
 * @param current_regs Register des aktuellen Tasks
 * @return Register des nächsten Tasks
 */
registers_t* task_switch(registers_t *current_regs);

/**
 * task_sleep - Lässt den aktuellen Task für X Ticks schlafen
 *
 * @param ticks Anzahl Timer-Ticks
 */
void task_sleep(uint64_t ticks);

/**
 * task_exit - Beendet den aktuellen Task
 */
void task_exit(void);

/**
 * task_count - Gibt Anzahl der Tasks zurück
 *
 * @return Anzahl Tasks
 */
int task_count(void);

/**
 * task_get_by_index - Gibt Task an Index zurück (für tasks-Command)
 *
 * @param index Index
 * @return Task oder NULL
 */
task_t* task_get_by_index(int index);

#endif /* KIOS_TASK_H */
