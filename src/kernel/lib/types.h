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
#define KIOS_TYPES_H


// Eigene Standard-Typen (ohne libc)
typedef unsigned char      uint8_t;
typedef signed char        int8_t;
typedef unsigned short     uint16_t;
typedef signed short       int16_t;
typedef unsigned int       uint32_t;
typedef signed int         int32_t;
typedef unsigned long long uint64_t;
typedef signed long long   int64_t;

typedef uint64_t            size_t;
typedef int64_t             ssize_t;
typedef uint64_t            uintptr_t;
typedef int64_t             intptr_t;
typedef enum { false = 0, true = 1 } bool;
#define NULL ((void*)0)

#endif /* KIOS_TYPES_H */