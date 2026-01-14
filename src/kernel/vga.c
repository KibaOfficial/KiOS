// Copyright (c) 2026 KibaOfficial
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "vga.h"

/* Globale VGA-Variablen - nur hier definiert! */
uint16_t* vga_buffer = VGA_BUFFER;
int vga_cursor_x = 0;
int vga_cursor_y = 0;
uint8_t vga_color = 0x0F;  /* Weiß auf Schwarz */
