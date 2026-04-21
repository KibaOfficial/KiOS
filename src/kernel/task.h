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

typedef enum {
    TASK_STATE_READY,
    TASK_STATE_RUNNING,
    TASK_STATE_BLOCKED,
    TASK_STATE_SLEEPING,
    TASK_STATE_ZOMBIE
} task_state_t;

#define TASK_NAME_MAX 32
#define MAX_TASKS 64

typedef struct task {
    uint32_t pid;
    char name[TASK_NAME_MAX];
    task_state_t state;

    registers_t *regs;          // aktueller Zustand (vom Scheduler aktualisiert)
    registers_t *initial_regs;  // initialer Frame (für Neustart nach exit)

    uint64_t stack_base;
    uint64_t stack_size;
    uint64_t sleep_until;
    int exit_code;

    struct task *next;
} task_t;

void task_init(void);
task_t* task_create(const char *name, void (*entry)(void), uint64_t stack_size);
task_t* task_get_current(void);
registers_t* task_switch(registers_t *current_regs);
void task_sleep(uint64_t ticks);
void task_set_shell(task_t *t);
task_t* task_get_next_ready(void);
__attribute__((noreturn)) void task_exit_current(int code);
void task_exit(void);
int task_count(void);
task_t* task_get_by_index(int index);
extern void task_restore(void *regs);

#endif /* KIOS_TASK_H */