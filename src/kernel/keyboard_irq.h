// Copyright (c) 2026 KibaOfficial
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

/*
 * KiOS - Keyboard Interrupt Handler Header
 */

#ifndef KIOS_KEYBOARD_IRQ_H
#define KIOS_KEYBOARD_IRQ_H

#include "types.h"

/* Funktionen */
void keyboard_irq_init(void);
int kb_irq_has_scancode(void);
uint8_t kb_irq_get_scancode(void);

/* High-level Funktionen (nutzen keyboard.h für ASCII-Konvertierung) */
char kb_getchar_irq(void);
char kb_try_getchar_irq(void);

#endif /* KIOS_KEYBOARD_IRQ_H */
