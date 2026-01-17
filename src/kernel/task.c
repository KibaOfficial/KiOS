/**
 * Copyright (c) 2026 KibaOfficial
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
#include "task.h"
#include "mm/heap.h"
#include "pit.h"
#include "vga.h"
#include "string.h"

/* =============================================================================
 * Globale Variablen
 * =============================================================================
 */

static task_t *task_list[MAX_TASKS];  // Array aller Tasks
static int task_count_val = 0;         // Anzahl Tasks
static task_t *current_task = NULL;    // Aktuell laufender Task
static uint32_t next_pid = 1;          // Nächste verfügbare PID

/* =============================================================================
 * Private Helper Functions
 * =============================================================================
 */

/**
 * task_wrapper - Wrapper-Funktion für Task-Entry-Point
 *
 * Wenn ein Task seine entry() Funktion verlässt, landen wir hier
 * und können den Task sauber beenden.
 */
static void task_wrapper(void) {
    // Task-Entry-Funktion aufrufen (wird vom Stack geholt)
    void (*entry)(void) = (void (*)(void))current_task;  // TODO: Wird noch richtig implementiert

    if (entry) {
        entry();
    }

    // Task ist fertig -> beenden
    task_exit();
}

/* =============================================================================
 * Public Functions
 * =============================================================================
 */

/**
 * task_init - Initialisiert das Task-Subsystem
 */
void task_init(void) {
    // Task-Liste leeren
    for (int i = 0; i < MAX_TASKS; i++) {
        task_list[i] = NULL;
    }

    task_count_val = 0;
    current_task = NULL;
    next_pid = 1;

    // Erstelle einen TCB für den aktuellen Kernel-Kontext (kernel_main)
    // Dieser wird zum "Idle Task" wenn der Scheduler aktiviert wird
    task_t *kernel_task = (task_t*)kmalloc(sizeof(task_t));
    if (kernel_task) {
        kernel_task->pid = 0;  // PID 0 für Kernel
        strncpy(kernel_task->name, "kernel_idle", TASK_NAME_MAX);
        kernel_task->state = TASK_STATE_RUNNING;
        kernel_task->stack_base = 0;  // Nutzt den Boot-Stack
        kernel_task->stack_size = 0;
        kernel_task->regs = NULL;  // Wird beim ersten Switch gesetzt
        kernel_task->sleep_until = 0;
        kernel_task->next = NULL;

        task_list[task_count_val++] = kernel_task;
        current_task = kernel_task;
    }

    // Task subsystem initialisiert - keine Debug-Ausgabe
}

/**
 * task_create - Erstellt einen neuen Task
 */
task_t* task_create(const char *name, void (*entry)(void), uint64_t stack_size) {
    if (task_count_val >= MAX_TASKS) {
        vga_println("[TASK] ERROR: Max tasks reached!");
        return NULL;
    }

    // TCB allokieren
    task_t *task = (task_t*)kmalloc(sizeof(task_t));
    if (!task) {
        vga_println("[TASK] ERROR: Failed to allocate TCB!");
        return NULL;
    }

    // Stack allokieren
    void *stack = kmalloc(stack_size);
    if (!stack) {
        vga_println("[TASK] ERROR: Failed to allocate stack!");
        kfree(task);
        return NULL;
    }

    // TCB initialisieren
    task->pid = next_pid++;
    strncpy(task->name, name, TASK_NAME_MAX);
    task->state = TASK_STATE_READY;
    task->stack_base = (uint64_t)stack;
    task->stack_size = stack_size;
    task->sleep_until = 0;
    task->next = NULL;

    // Register-State auf dem Stack vorbereiten
    // Stack wächst nach unten, also starten wir am Ende
    uint64_t stack_top = task->stack_base + stack_size;

    // WICHTIG: Stack auf 16 Bytes ausrichten (x86_64 Anforderung)
    stack_top &= ~0xFULL;  // Auf 16-Byte-Grenze abrunden

    // Platz für registers_t reservieren
    stack_top -= sizeof(registers_t);
    task->regs = (registers_t*)stack_top;

    // Register-State initialisieren
    registers_t *regs = task->regs;

    // Alle Register auf 0
    regs->rax = 0;
    regs->rbx = 0;
    regs->rcx = 0;
    regs->rdx = 0;
    regs->rsi = 0;
    regs->rdi = 0;
    regs->rbp = 0;
    regs->r8 = 0;
    regs->r9 = 0;
    regs->r10 = 0;
    regs->r11 = 0;
    regs->r12 = 0;
    regs->r13 = 0;
    regs->r14 = 0;
    regs->r15 = 0;

    // Instruction Pointer auf Entry-Funktion setzen
    regs->rip = (uint64_t)entry;

    // Stack Pointer
    regs->rsp = stack_top;

    // Segments (Kernel-Segmente)
    regs->cs = 0x08;   // Kernel Code Segment
    regs->ds = 0x10;   // Kernel Data Segment
    regs->es = 0x10;
    regs->fs = 0x10;
    regs->gs = 0x10;
    regs->ss = 0x10;   // Kernel Stack Segment

    // Flags (Interrupts enabled)
    regs->rflags = 0x202;

    // Interrupt Nummer und Error Code (nicht relevant für Task)
    regs->int_no = 0;
    regs->err_code = 0;

    // Task zur Liste hinzufügen
    task_list[task_count_val++] = task;

    // Wenn das der erste Task ist, als current setzen
    if (current_task == NULL) {
        current_task = task;
        task->state = TASK_STATE_RUNNING;
    }

    return task;
}

/**
 * task_get_current - Gibt aktuellen Task zurück
 */
task_t* task_get_current(void) {
    return current_task;
}

/**
 * task_switch - Wechselt zum nächsten Task (Round-Robin)
 */
registers_t* task_switch(registers_t *current_regs) {
    if (task_count_val == 0) {
        return current_regs;  // Keine Tasks
    }

    // Aktuellen Task-State speichern
    if (current_task && current_task->state == TASK_STATE_RUNNING) {
        current_task->regs = current_regs;
        current_task->state = TASK_STATE_READY;
    }

    // Nächsten READY Task finden (Round-Robin)
    task_t *next_task = NULL;
    int start_idx = 0;

    // Finde Index des aktuellen Tasks
    for (int i = 0; i < task_count_val; i++) {
        if (task_list[i] == current_task) {
            start_idx = (i + 1) % task_count_val;
            break;
        }
    }

    // Suche ab start_idx nach einem READY Task
    for (int i = 0; i < task_count_val; i++) {
        int idx = (start_idx + i) % task_count_val;
        task_t *t = task_list[idx];

        // PID 0 (kernel_idle) überspringen - läuft im HLT-Loop
        if (t->pid == 0) continue;

        if (t->state == TASK_STATE_READY) {
            next_task = t;
            break;
        }

        // Sleeping Tasks aufwecken, wenn Zeit abgelaufen
        if (t->state == TASK_STATE_SLEEPING) {
            if (pit_get_ticks() >= t->sleep_until) {
                t->state = TASK_STATE_READY;
                next_task = t;
                break;
            }
        }
    }

    // Kein READY Task gefunden? Behalte aktuellen
    if (!next_task) {
        if (current_task) {
            current_task->state = TASK_STATE_RUNNING;
        }
        return current_regs;
    }

    // Zu neuem Task wechseln
    current_task = next_task;
    current_task->state = TASK_STATE_RUNNING;

    return current_task->regs;
}

/**
 * task_sleep - Lässt Task schlafen
 */
void task_sleep(uint64_t ticks) {
    if (current_task) {
        current_task->state = TASK_STATE_SLEEPING;
        current_task->sleep_until = pit_get_ticks() + ticks;
        // Context Switch wird beim nächsten Timer-Tick passieren
    }
}

/**
 * task_exit - Beendet aktuellen Task
 */
void task_exit(void) {
    if (current_task) {
        current_task->state = TASK_STATE_ZOMBIE;
        // TODO: Stack und TCB freigeben
        // TODO: Aus task_list entfernen
        // Erstmal nur als Zombie markieren
    }
}

/**
 * task_count - Gibt Anzahl Tasks zurück
 */
int task_count(void) {
    return task_count_val;
}

/**
 * task_get_by_index - Gibt Task an Index zurück
 */
task_t* task_get_by_index(int index) {
    if (index >= 0 && index < task_count_val) {
        return task_list[index];
    }
    return NULL;
}
