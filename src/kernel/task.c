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

static task_t *task_list[MAX_TASKS];
static int task_count_val = 0;
static task_t *current_task = NULL;
static task_t *shell_task = NULL;
static uint32_t next_pid = 1;

task_t *task_get_next_ready(void);
extern void task_restore(void *regs);

void task_init(void)
{
    for (int i = 0; i < MAX_TASKS; i++)
        task_list[i] = NULL;
    task_count_val = 0;
    current_task = NULL;
    shell_task = NULL;
    next_pid = 1;

    task_t *kernel_task = (task_t *)kmalloc(sizeof(task_t));
    if (kernel_task)
    {
        kernel_task->pid = 0;
        strncpy(kernel_task->name, "kernel_idle", TASK_NAME_MAX);
        kernel_task->state = TASK_STATE_RUNNING;
        kernel_task->stack_base = 0;
        kernel_task->stack_size = 0;
        kernel_task->regs = NULL;
        kernel_task->initial_regs = NULL;
        kernel_task->sleep_until = 0;
        kernel_task->exit_code = 0;
        kernel_task->next = NULL;
        task_list[task_count_val++] = kernel_task;
        current_task = kernel_task;
    }
}

task_t *task_create(const char *name, void (*entry)(void), uint64_t stack_size)
{
    if (task_count_val >= MAX_TASKS) { vga_println("[TASK] ERROR: Max tasks reached!"); return NULL; }

    task_t *task = (task_t *)kmalloc(sizeof(task_t));
    if (!task) { vga_println("[TASK] ERROR: Failed to allocate TCB!"); return NULL; }

    void *stack = kmalloc(stack_size);
    if (!stack) { vga_println("[TASK] ERROR: Failed to allocate stack!"); kfree(task); return NULL; }

    task->pid = next_pid++;
    strncpy(task->name, name, TASK_NAME_MAX);
    task->state = TASK_STATE_READY;
    task->stack_base = (uint64_t)stack;
    task->stack_size = stack_size;
    task->sleep_until = 0;
    task->exit_code = 0;
    task->next = NULL;

    uint64_t stack_top = (task->stack_base + stack_size) & ~0xFULL;
    stack_top -= sizeof(registers_t);
    task->regs = (registers_t *)stack_top;

    // Register initialisieren
    registers_t *regs = task->regs;
    regs->rax = 0; regs->rbx = 0; regs->rcx = 0; regs->rdx = 0;
    regs->rsi = 0; regs->rdi = 0; regs->rbp = 0;
    regs->r8  = 0; regs->r9  = 0; regs->r10 = 0; regs->r11 = 0;
    regs->r12 = 0; regs->r13 = 0; regs->r14 = 0; regs->r15 = 0;
    regs->rip    = (uint64_t)entry;
    regs->rsp    = stack_top + sizeof(registers_t);
    regs->cs     = 0x08;
    regs->ss     = 0x10;
    regs->ds     = 0x10;
    regs->es     = 0x10;
    regs->fs     = 0x10;
    regs->gs     = 0x10;
    regs->rflags = 0x202;
    regs->int_no   = 0;
    regs->err_code = 0;

    // JETZT erst initial_regs kopieren - nachdem alles korrekt initialisiert ist!
    task->initial_regs = (registers_t *)kmalloc(sizeof(registers_t));
    *task->initial_regs = *task->regs;

    task_list[task_count_val++] = task;
    if (current_task == NULL) { current_task = task; task->state = TASK_STATE_RUNNING; }
    return task;
}

void task_set_shell(task_t *t) { shell_task = t; }

task_t *task_get_next_ready(void)
{
    if (shell_task)
    {
        if (shell_task->state == TASK_STATE_READY ||
            shell_task->state == TASK_STATE_RUNNING)
        {
            shell_task->state = TASK_STATE_READY;
            return shell_task;
        }
    }
    for (int i = 0; i < task_count_val; i++)
    {
        task_t *t = task_list[i];
        if (t && t != current_task && t->state == TASK_STATE_READY) return t;
    }
    return NULL;
}

task_t *task_get_current(void) { return current_task; }

registers_t *task_switch(registers_t *current_regs)
{
    if (task_count_val == 0) return current_regs;

    if (current_task && current_task->state == TASK_STATE_RUNNING)
    {
        current_task->regs = current_regs;
        current_task->state = TASK_STATE_READY;
    }

    task_t *next_task = NULL;
    int start_idx = 0;
    for (int i = 0; i < task_count_val; i++)
    {
        if (task_list[i] == current_task) { start_idx = (i + 1) % task_count_val; break; }
    }

    for (int i = 0; i < task_count_val; i++)
    {
        int idx = (start_idx + i) % task_count_val;
        task_t *t = task_list[idx];
        if (t->pid == 0) continue;
        if (t->state == TASK_STATE_ZOMBIE) continue;
        if (t->state == TASK_STATE_READY) { next_task = t; break; }
        if (t->state == TASK_STATE_SLEEPING && pit_get_ticks() >= t->sleep_until)
        {
            t->state = TASK_STATE_READY;
            next_task = t;
            break;
        }
    }

    if (!next_task)
    {
        if (current_task) current_task->state = TASK_STATE_RUNNING;
        return current_regs;
    }

    current_task = next_task;
    current_task->state = TASK_STATE_RUNNING;
    return current_task->regs;
}

void task_sleep(uint64_t ticks)
{
    if (current_task)
    {
        current_task->state = TASK_STATE_SLEEPING;
        current_task->sleep_until = pit_get_ticks() + ticks;
    }
}

__attribute__((noreturn)) void task_exit_current(int code)
{
    if (current_task && current_task != shell_task)
    {
        current_task->state = TASK_STATE_ZOMBIE;
        current_task->exit_code = code;
    }

    task_t *next = task_get_next_ready();
    if (!next)
    {
        vga_println("[EXIT] WARN: no next task, falling back to kernel_idle");
        next = task_list[0];
    }
    // else
    // {
    //     vga_print("[EXIT] switching to: ");
    //     vga_println(next->name);
    // }

    if (!next->initial_regs)
    {
        vga_println("[EXIT] FATAL: next->initial_regs is NULL!");
        while (1) { asm volatile("hlt"); }
    }

    next->state = TASK_STATE_RUNNING;
    current_task = next;
    task_restore(next->initial_regs);

    while (1) { asm volatile("hlt"); }
}

void task_exit(void) { task_exit_current(0); }

int task_count(void) { return task_count_val; }

task_t *task_get_by_index(int index)
{
    if (index >= 0 && index < task_count_val) return task_list[index];
    return NULL;
}