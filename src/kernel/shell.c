#include "shell.h"
#include "vga.h"
#include "keyboard.h"
#include "string.h"
#include "io.h"

static char shell_history[SHELL_HISTORY_SIZE][SHELL_BUFFER_SIZE];
static int shell_history_count = 0;
static int shell_history_index = 0;
static char shell_buffer[SHELL_BUFFER_SIZE];
static int shell_buffer_pos = 0;

void shell_print_prompt(void) {
    vga_print_colored("kiba", VGA_LIGHT_GREEN, VGA_BLACK);
    vga_print_colored("@", VGA_WHITE, VGA_BLACK);
    vga_print_colored("KiOS", VGA_LIGHT_CYAN, VGA_BLACK);
    vga_print_colored("> ", VGA_WHITE, VGA_BLACK);
}

void shell_add_to_history(const char* cmd) {
    if (strlen(cmd) == 0) return;
    if (shell_history_count > 0 && strcmp(cmd, shell_history[(shell_history_count - 1) % SHELL_HISTORY_SIZE]) == 0) {
        return;
    }
    strcpy(shell_history[shell_history_count % SHELL_HISTORY_SIZE], cmd);
    shell_history_count++;
    shell_history_index = shell_history_count;
}

void shell_clear_line(void) {
    while (shell_buffer_pos > 0) {
        vga_putchar('\b');
        shell_buffer_pos--;
    }
    shell_buffer[0] = '\0';
}

static void shell_execute(const char* cmd) {
    while (*cmd == ' ') cmd++;
    if (*cmd == '\0') return;
    const char* args = cmd;
    while (*args && *args != ' ') args++;
    size_t cmd_len = args - cmd;
    while (*args == ' ') args++;
    for (int i = 0; i < shell_commands_count; i++) {
        if (strncmp(cmd, shell_commands[i].name, cmd_len) == 0 && strlen(shell_commands[i].name) == cmd_len) {
            shell_commands[i].func(args);
            return;
        }
    }
    vga_print_colored("Unknown command: ", VGA_LIGHT_RED, VGA_BLACK);
    for (size_t i = 0; i < cmd_len; i++) {
        vga_putchar(cmd[i]);
    }
    vga_println("");
    vga_println("Type 'help' for available commands.");
}

const char* shell_readline(void) {
    shell_buffer_pos = 0;
    shell_buffer[0] = '\0';
    shell_history_index = shell_history_count;
    while (1) {
        char c = kb_getchar();  /* Zurück zu Polling (IRQ hat noch Bugs) */
        switch (c) {
            case '\n':
                vga_putchar('\n');
                shell_buffer[shell_buffer_pos] = '\0';
                shell_add_to_history(shell_buffer);
                return shell_buffer;
            case '\b':
                if (shell_buffer_pos > 0) {
                    shell_buffer_pos--;
                    shell_buffer[shell_buffer_pos] = '\0';
                    vga_putchar('\b');
                }
                break;
            case 27:
                shell_clear_line();
                break;
            default:
                if (c >= ' ' && shell_buffer_pos < SHELL_BUFFER_SIZE - 1) {
                    shell_buffer[shell_buffer_pos++] = c;
                    shell_buffer[shell_buffer_pos] = '\0';
                    vga_putchar(c);
                }
                break;
        }
    }
}

void shell_run(void) {
    while (1) {
        shell_print_prompt();
        const char* cmd = shell_readline();
        shell_execute(cmd);
    }
}
