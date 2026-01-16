#ifndef KIOS_COMMANDS_H
#define KIOS_COMMANDS_H

#include "types.h"

/* Funktionsprototypen für alle Shell-Kommandos */
void cmd_help(const char* args);
void cmd_clear(const char* args);
void cmd_info(const char* args);
void cmd_echo(const char* args);
void cmd_color(const char* args);
void cmd_reboot(const char* args);
void cmd_halt(const char* args);
void cmd_mem(const char* args);
void cmd_time(const char* args);
void cmd_netconf(const char* args);
void cmd_shutdown(const char* args);


void cmd_fault(const char* args);
void cmd_mmap(const char* args);
void cmd_vmtest(const char* args);
void cmd_meminfo(const char* args);
void cmd_memtest(const char* args);

#endif /* KIOS_COMMANDS_H */
