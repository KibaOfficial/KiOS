/**
 * Copyright (c) 2026 KibaOfficial
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
#include "../shell.h"
#include "../vga.h"
#include "../task.h"

void cmd_tasks(const char* args) {
    (void)args;

    int count = task_count();

    if (count == 0) {
        vga_println("No tasks running.");
        return;
    }

    vga_println("PID  State      Name");
    vga_println("---  ---------  --------");

    for (int i = 0; i < count; i++) {
        task_t *task = task_get_by_index(i);
        if (!task) continue;

        // PID
        vga_print_dec(task->pid);
        vga_print("    ");

        // State
        const char *state_str = "???";
        switch (task->state) {
            case TASK_STATE_READY:    state_str = "READY    "; break;
            case TASK_STATE_RUNNING:  state_str = "RUNNING  "; break;
            case TASK_STATE_BLOCKED:  state_str = "BLOCKED  "; break;
            case TASK_STATE_SLEEPING: state_str = "SLEEPING "; break;
            case TASK_STATE_ZOMBIE:   state_str = "ZOMBIE   "; break;
        }
        vga_print(state_str);
        vga_print(" ");

        // Name
        vga_println(task->name);
    }
}
