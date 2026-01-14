// Copyright (c) 2026 KibaOfficial
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

/*
 * KiOS - Kernel Type Definitions
 * 
 * Da wir keine Standardbibliothek haben, definieren wir unsere eigenen Typen.
 * Diese entsprechen den üblichen stdint.h Typen.
 */

#ifndef KIOS_TYPES_H
#ifndef KIOS_TYPES_H
#define KIOS_TYPES_H

#include <stdint.h>

#endif /* KIOS_TYPES_H */
typedef uint64_t            size_t;
typedef int64_t             ssize_t;

/* Pointer-sized Integer */
typedef uint64_t            uintptr_t;
typedef int64_t             intptr_t;

/* Boolean */
typedef enum { false = 0, true = 1 } bool;

/* NULL Pointer */
#define NULL ((void*)0)


#endif /* KIOS_TYPES_H */
