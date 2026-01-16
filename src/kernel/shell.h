// Copyright (c) 2026 KibaOfficial
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

/*
 * KiOS - Shell
 *
 * Eine einfache interaktive Shell mit grundlegenden Befehlen.
 */

#ifndef KIOS_SHELL_H
#define KIOS_SHELL_H

#include "types.h"
#include "vga.h"
#include "keyboard.h"
#include "string.h"
#include "io.h"

/* Shell Konstanten */
#define SHELL_BUFFER_SIZE 256
#define SHELL_HISTORY_SIZE 10


/* Funktionsprototypen für shell.c */
void shell_print_prompt(void);
void shell_add_to_history(const char* cmd);
void shell_clear_line(void);
const char* shell_readline(void);
void shell_run(void);


typedef void (*shell_cmd_fn)(const char* args);

typedef struct {
    const char* name;
    shell_cmd_fn func;
    const char* help;
} shell_command_t;

/* Funktionsprototypen für Kommandos (werden in commands.h bereitgestellt) */
#include "commands.h"

extern const shell_command_t shell_commands[];
extern const int shell_commands_count;

#endif /* KIOS_SHELL_H */
