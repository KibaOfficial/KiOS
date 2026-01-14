// Copyright (c) 2026 KibaOfficial
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

/*
 * KiOS - VGA Text Mode Driver
 *
 * VGA Text Mode ist der einfachste Weg, Text auf den Bildschirm zu bringen.
 * Der Framebuffer liegt bei 0xB8000 und ist 80x25 Zeichen groß.
 * Jedes Zeichen besteht aus 2 Bytes: [ASCII-Code][Attribut]
 *
 * Attribut-Byte:
 *   Bits 0-3: Vordergrundfarbe (0-15)
 *   Bits 4-6: Hintergrundfarbe (0-7)
 *   Bit 7:    Blink (wenn aktiviert)
 */

#ifndef KIOS_VGA_H
#define KIOS_VGA_H

#include "types.h"
#include "io.h"
#include "string.h"

/* VGA Konstanten */
#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_BUFFER  ((uint16_t*)0xB8000)

/* VGA I/O Ports für Cursor */
#define VGA_CTRL_PORT   0x3D4
#define VGA_DATA_PORT   0x3D5

/* Farben */
typedef enum {
    VGA_BLACK         = 0,
    VGA_BLUE          = 1,
    VGA_GREEN         = 2,
    VGA_CYAN          = 3,
    VGA_RED           = 4,
    VGA_MAGENTA       = 5,
    VGA_BROWN         = 6,
    VGA_LIGHT_GRAY    = 7,
    VGA_DARK_GRAY     = 8,
    VGA_LIGHT_BLUE    = 9,
    VGA_LIGHT_GREEN   = 10,
    VGA_LIGHT_CYAN    = 11,
    VGA_LIGHT_RED     = 12,
    VGA_LIGHT_MAGENTA = 13,
    VGA_YELLOW        = 14,
    VGA_WHITE         = 15
} vga_color_t;

/* Globale Variablen - definiert in vga.c */
extern uint16_t* vga_buffer;
extern int vga_cursor_x;
extern int vga_cursor_y;
extern uint8_t vga_color;

/*
 * vga_make_color - Erstellt ein Farb-Attribut
 */
static inline uint8_t vga_make_color(vga_color_t fg, vga_color_t bg) {
    return (uint8_t)(fg | (bg << 4));
}

/*
 * vga_make_entry - Erstellt einen VGA-Eintrag (Zeichen + Farbe)
 */
static inline uint16_t vga_make_entry(char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

/*
 * vga_set_color - Setzt die aktuelle Farbe
 */
static inline void vga_set_color(vga_color_t fg, vga_color_t bg) {
    vga_color = vga_make_color(fg, bg);
}

/*
 * vga_update_cursor - Aktualisiert die Hardware-Cursor-Position
 */
static inline void vga_update_cursor(void) {
    uint16_t pos = vga_cursor_y * VGA_WIDTH + vga_cursor_x;
    
    /* Cursor Position setzen (High und Low Byte) */
    outb(VGA_CTRL_PORT, 0x0F);          /* Low Byte Register */
    outb(VGA_DATA_PORT, (uint8_t)(pos & 0xFF));
    outb(VGA_CTRL_PORT, 0x0E);          /* High Byte Register */
    outb(VGA_DATA_PORT, (uint8_t)((pos >> 8) & 0xFF));
}

/*
 * vga_scroll - Scrollt den Bildschirm um eine Zeile nach oben
 */
static inline void vga_scroll(void) {
    /* Alles eine Zeile nach oben kopieren */
    for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
        vga_buffer[i] = vga_buffer[i + VGA_WIDTH];
    }
    
    /* Letzte Zeile leeren */
    uint16_t blank = vga_make_entry(' ', vga_color);
    for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++) {
        vga_buffer[i] = blank;
    }
    
    vga_cursor_y = VGA_HEIGHT - 1;
}

/*
 * vga_clear - Löscht den Bildschirm
 */
static inline void vga_clear(void) {
    /* Erstmal sicherstellen dass wir den richtigen Buffer nutzen */
    vga_buffer = VGA_BUFFER;
    
    /* Bildschirm löschen */
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = vga_make_entry(' ', VGA_BLACK);
    }

    /* Cursor zurücksetzen */
    vga_cursor_x = 0;
    vga_cursor_y = 0;
    vga_update_cursor();
}

/*
 * vga_putchar_at - Zeichen an bestimmter Position setzen
 */
static inline void vga_putchar_at(char c, int x, int y, uint8_t color) {
    /* Bounds check - verhindert Schreiben außerhalb des VGA-Buffers */
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
        vga_buffer[y * VGA_WIDTH + x] = vga_make_entry(c, color);
    }
}

/*
 * vga_putchar - Zeichen an aktueller Cursor-Position ausgeben
 */
static inline void vga_putchar(char c) {
    /* Scrollen wenn nötig VOR dem Schreiben */
    if (vga_cursor_y >= VGA_HEIGHT) {
        vga_scroll();
    }

    switch (c) {
        case '\n':
            vga_cursor_x = 0;
            vga_cursor_y++;
            break;

        case '\r':
            vga_cursor_x = 0;
            break;

        case '\t':
            /* Tab = nächstes 8er-Vielfaches */
            vga_cursor_x = (vga_cursor_x + 8) & ~7;
            break;

        case '\b':
            /* Backspace */
            if (vga_cursor_x > 0) {
                vga_cursor_x--;
                vga_putchar_at(' ', vga_cursor_x, vga_cursor_y, vga_color);
            }
            break;

        default:
            if (c >= ' ') {
                vga_putchar_at(c, vga_cursor_x, vga_cursor_y, vga_color);
                vga_cursor_x++;
            }
            break;
    }

    /* Zeilenumbruch wenn nötig */
    if (vga_cursor_x >= VGA_WIDTH) {
        vga_cursor_x = 0;
        vga_cursor_y++;
    }

    /* Nochmal scrollen wenn nötig (nach Cursor-Update) */
    if (vga_cursor_y >= VGA_HEIGHT) {
        vga_scroll();
    }

    vga_update_cursor();
}

/*
 * vga_print - String ausgeben
 */
static inline void vga_print(const char* str) {
    while (*str) {
        vga_putchar(*str++);
    }
}

/*
 * vga_println - String mit Zeilenumbruch ausgeben
 */
static inline void vga_println(const char* str) {
    vga_print(str);
    vga_putchar('\n');
}

/*
 * vga_print_hex - Zahl als Hex ausgeben
 */
static inline void vga_print_hex(uint64_t value) {
    const char* hex_chars = "0123456789ABCDEF";
    char buffer[19]; /* "0x" + 16 Hex-Zeichen + '\0' */
    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[18] = '\0';
    
    for (int i = 0; i < 16; i++) {
        buffer[17 - i] = hex_chars[value & 0xF];
        value >>= 4;
    }
    
    /* Führende Nullen überspringen (aber mindestens eine Ziffer) */
    const char* ptr = buffer + 2;
    while (*ptr == '0' && *(ptr + 1) != '\0') {
        ptr++;
    }
    
    vga_print("0x");
    vga_print(ptr);
}

/*
 * vga_print_dec - Zahl als Dezimal ausgeben
 */
static inline void vga_print_dec(int64_t value) {
    if (value < 0) {
        vga_putchar('-');
        value = -value;
    }
    
    if (value == 0) {
        vga_putchar('0');
        return;
    }
    
    char buffer[21];
    int i = 0;
    
    while (value > 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }
    
    while (i > 0) {
        vga_putchar(buffer[--i]);
    }
}

/*
 * vga_print_colored - String in bestimmter Farbe ausgeben
 */
static inline void vga_print_colored(const char* str, vga_color_t fg, vga_color_t bg) {
    uint8_t old_color = vga_color;
    vga_color = vga_make_color(fg, bg);
    vga_print(str);
    vga_color = old_color;
}

#endif /* KIOS_VGA_H */