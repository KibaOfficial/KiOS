/*
 * KiOS - String Functions
 *
 * Da wir keine Standardbibliothek haben, implementieren wir
 * die nötigsten String-Funktionen selbst.
 */

#ifndef KIOS_STRING_H
#define KIOS_STRING_H

#include "types.h"

/*
 * strlen - Länge eines null-terminierten Strings
 */
static inline size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}

/*
 * strcmp - Zwei Strings vergleichen
 * 
 * @return: 0 wenn gleich, <0 wenn s1<s2, >0 wenn s1>s2
 */
static inline int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

/*
 * strncmp - Ersten n Zeichen vergleichen
 */
static inline int strncmp(const char* s1, const char* s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) return 0;
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

/*
 * strcpy - String kopieren
 */
static inline char* strcpy(char* dest, const char* src) {
    char* ret = dest;
    while ((*dest++ = *src++));
    return ret;
}

/*
 * strncpy - Maximal n Zeichen kopieren
 */
static inline char* strncpy(char* dest, const char* src, size_t n) {
    char* ret = dest;
    while (n && (*dest++ = *src++)) {
        n--;
    }
    while (n--) {
        *dest++ = '\0';
    }
    return ret;
}

/*
 * memset - Speicher mit einem Wert füllen
 */
static inline void* memset(void* dest, int val, size_t count) {
    unsigned char* ptr = (unsigned char*)dest;
    while (count--) {
        *ptr++ = (unsigned char)val;
    }
    return dest;
}

/*
 * memcpy - Speicher kopieren
 */
static inline void* memcpy(void* dest, const void* src, size_t count) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    while (count--) {
        *d++ = *s++;
    }
    return dest;
}

/*
 * memcmp - Speicher vergleichen
 */
static inline int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    while (n--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return 0;
}

/*
 * memmove - Speicher kopieren (überlappende Bereiche sicher)
 */
static inline void* memmove(void* dest, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    
    if (d < s) {
        while (n--) {
            *d++ = *s++;
        }
    } else {
        d += n;
        s += n;
        while (n--) {
            *--d = *--s;
        }
    }
    return dest;
}

#endif /* KIOS_STRING_H */
