/**
 * Copyright (c) 2026 KibaOfficial
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include "../commands.h"
#include "../vga.h"

void cmd_netconf(const char* args) {
  /**
   * NetConf ist das KiOS equivalent zu ifconfig/iwconfig.
   * Derzeit werden keine Netzwerkschnittstellen unterstützt,
   * daher gibt dieser Befehl nur eine entsprechende Meldung aus.
   */
  (void)args;
  vga_println("No network interfaces configured.");
}