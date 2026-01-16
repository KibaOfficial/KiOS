// Copyright (c) 2026 KibaOfficial
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

/*
 * KiOS - PS/2 Keyboard Driver
 *
 * Die PS/2 Tastatur kommuniziert über I/O Ports:
 *   0x60 - Data Port (Scancodes lesen)
 *   0x64 - Status/Command Port
 *
 * Die Tastatur sendet "Scancodes" - jede Taste hat eine Nummer.
 * Wir müssen diese in ASCII-Zeichen umwandeln.
 */

#ifndef KIOS_KEYBOARD_H
#define KIOS_KEYBOARD_H

#include "types.h"
#include "io.h"

/* I/O Ports */
#define KB_DATA_PORT    0x60
#define KB_STATUS_PORT  0x64
#define KB_CMD_PORT     0x64

/* Status Register Bits */
#define KB_STATUS_OUTPUT_FULL  0x01   /* Daten zum Lesen bereit */
#define KB_STATUS_INPUT_FULL   0x02   /* Tastatur wartet auf Daten */

/* Spezielle Scancodes */
#define SC_RELEASE      0x80   /* Bit gesetzt bei Key Release */
#define SC_ESCAPE       0x01
#define SC_BACKSPACE    0x0E
#define SC_TAB          0x0F
#define SC_ENTER        0x1C
#define SC_LCTRL        0x1D
#define SC_LSHIFT       0x2A
#define SC_RSHIFT       0x36
#define SC_LALT         0x38
#define SC_CAPSLOCK     0x3A
#define SC_F1           0x3B
#define SC_F12          0x58

/* Modifier States */
static bool kb_shift_pressed = false;
static bool kb_ctrl_pressed = false;
static bool kb_alt_pressed = false;
static bool kb_capslock_on = false;

/*
 * US Keyboard Layout - Scancode zu ASCII Tabelle
 * Index = Scancode, Wert = ASCII Zeichen (0 = keine Zuordnung)
 */
static const char scancode_to_ascii_lower[128] = {
    0,    27,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,    '\\','z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*',  0,   ' ', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0
};

/* Shift gedrückt */
static const char scancode_to_ascii_upper[128] = {
    0,    27,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0,    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*',  0,   ' ', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0
};

/*
 * kb_wait_output - Wartet bis Daten zum Lesen bereit sind
 * Mit Timeout um Hängen zu vermeiden
 */
static bool kb_wait_output(void) {
    int timeout = 100000;
    while (timeout-- > 0) {
        if (inb(KB_STATUS_PORT) & KB_STATUS_OUTPUT_FULL) {
            return true;
        }
    }
    return false;
}

/*
 * kb_read_scancode - Liest einen Scancode (blockierend mit Timeout)
 */
static uint8_t kb_read_scancode(void) {
    if (kb_wait_output()) {
        return inb(KB_DATA_PORT);
    }
    return 0;
}

/*
 * kb_try_read_scancode - Versucht einen Scancode zu lesen (nicht blockierend)
 *
 * @return: Scancode oder 0 wenn nichts verfügbar
 */
static uint8_t kb_try_read_scancode(void) {
    if (inb(KB_STATUS_PORT) & KB_STATUS_OUTPUT_FULL) {
        return inb(KB_DATA_PORT);
    }
    return 0;
}

/*
 * kb_scancode_to_ascii - Wandelt Scancode in ASCII um
 *
 * @scancode: Der Scancode
 * @return: ASCII-Zeichen oder 0 bei Spezial-/Modifier-Tasten
 */
static char kb_scancode_to_ascii(uint8_t scancode) {
    /* Key Release ignorieren (aber Modifier-State aktualisieren) */
    if (scancode & SC_RELEASE) {
        uint8_t key = scancode & ~SC_RELEASE;
        
        if (key == SC_LSHIFT || key == SC_RSHIFT) {
            kb_shift_pressed = false;
        } else if (key == SC_LCTRL) {
            kb_ctrl_pressed = false;
        } else if (key == SC_LALT) {
            kb_alt_pressed = false;
        }
        
        return 0;
    }
    
    /* Modifier-Tasten verarbeiten */
    switch (scancode) {
        case SC_LSHIFT:
        case SC_RSHIFT:
            kb_shift_pressed = true;
            return 0;
            
        case SC_LCTRL:
            kb_ctrl_pressed = true;
            return 0;
            
        case SC_LALT:
            kb_alt_pressed = true;
            return 0;
            
        case SC_CAPSLOCK:
            kb_capslock_on = !kb_capslock_on;
            return 0;
    }
    
    /* Normale Tasten */
    if (scancode < 128) {
        bool use_upper = kb_shift_pressed;
        
        /* CapsLock invertiert nur Buchstaben */
        char lower = scancode_to_ascii_lower[scancode];
        if (kb_capslock_on && lower >= 'a' && lower <= 'z') {
            use_upper = !use_upper;
        }
        
        if (use_upper) {
            return scancode_to_ascii_upper[scancode];
        } else {
            return scancode_to_ascii_lower[scancode];
        }
    }
    
    return 0;
}

/*
 * kb_getchar - Liest ein Zeichen (blockierend)
 *
 * Wartet bis eine "echte" Taste gedrückt wird (keine Modifier alleine).
 */
static char kb_getchar(void) {
    while (1) {
        uint8_t scancode = kb_read_scancode();
        char c = kb_scancode_to_ascii(scancode);
        
        if (c != 0) {
            return c;
        }
    }
}

/*
 * kb_try_getchar - Versucht ein Zeichen zu lesen (nicht blockierend)
 *
 * @return: Zeichen oder 0 wenn nichts verfügbar
 */
static char kb_try_getchar(void) {
    uint8_t scancode = kb_try_read_scancode();
    if (scancode == 0) {
        return 0;
    }
    return kb_scancode_to_ascii(scancode);
}

/*
 * kb_is_shift_pressed - Prüft ob Shift gedrückt ist
 */
static inline bool kb_is_shift_pressed(void) {
    return kb_shift_pressed;
}

/*
 * kb_is_ctrl_pressed - Prüft ob Ctrl gedrückt ist
 */
static inline bool kb_is_ctrl_pressed(void) {
    return kb_ctrl_pressed;
}

/*
 * kb_is_alt_pressed - Prüft ob Alt gedrückt ist
 */
static inline bool kb_is_alt_pressed(void) {
    return kb_alt_pressed;
}

#endif /* KIOS_KEYBOARD_H */