// Copyright (c) 2026 KibaOfficial
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "shell.h"
#include "commands.h"

/* Shell Commands Array - definiert alle verfügbaren Kommandos */
const shell_command_t shell_commands[] = {
    {"help",    cmd_help,    "Show this help message"},
    {"clear",   cmd_clear,   "Clear the screen"},
    {"info",    cmd_info,    "Show system information"},
    {"echo",    cmd_echo,    "Echo arguments to screen"},
    {"color",   cmd_color,   "Change text color (usage: color <fg> <bg>)"},
    {"mem",     cmd_mem,     "Show memory information"},
    {"mmap",    cmd_mmap,    "Show physical memory map"},
    {"meminfo", cmd_meminfo, "Show detailed memory statistics"},
    {"memtest", cmd_memtest, "Run comprehensive memory stress tests"},
    {"time",    cmd_time,    "Show current time"},
    {"uptime",  cmd_uptime,  "Show system uptime"},
    {"tasks",   cmd_tasks,   "List all running tasks"},
    {"reboot",  cmd_reboot,  "Reboot the system"},
    {"shutdown",cmd_shutdown, "Shutdown the system"},
    {"halt",    cmd_halt,    "Halt the system"},
    {"netconf", cmd_netconf,  "Configure network interface"},
    {"fault",   cmd_fault,   "Trigger CPU exceptions for testing (usage: fault <div0|ud|pf>)"},
    {"vmtest",  cmd_vmtest,  "Test Virtual Memory Manager"},
    {"usertest",cmd_usertest,"Test Ring 3 / User Mode transition"}
};

const int shell_commands_count = sizeof(shell_commands) / sizeof(shell_commands[0]);
