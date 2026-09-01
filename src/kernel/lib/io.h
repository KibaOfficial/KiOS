/*
 * KiOS - I/O Port Functions
 *
 * In x86 gibt es zwei Wege mit Hardware zu kommunizieren:
 * 1. Memory-Mapped I/O (MMIO) - Hardware-Register im Speicher abgebildet
 * 2. Port I/O - Separater I/O-Adressraum, Zugriff über IN/OUT Instruktionen
 *
 * Viele Legacy-Geräte (Keyboard, PIC, etc.) nutzen Port I/O.
 */

#ifndef KIOS_IO_H
#define KIOS_IO_H

#include "types.h"

/*
 * outb - Schreibt ein Byte an einen I/O Port
 * 
 * @port: Der I/O Port (0-65535)
 * @value: Das zu schreibende Byte
 *
 * Das "volatile" verhindert, dass der Compiler die Instruktion wegoptimiert.
 * Die "a" Constraint sagt: value muss in AL sein.
 * Die "Nd" Constraint sagt: port kann ein Immediate (0-255) oder in DX sein.
 */
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

/*
 * inb - Liest ein Byte von einem I/O Port
 *
 * @port: Der I/O Port
 * @return: Das gelesene Byte
 */
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/*
 * outw - Schreibt ein Word (16 Bit) an einen I/O Port
 */
static inline void outw(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

/*
 * inw - Liest ein Word von einem I/O Port
 */
static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/*
 * outl - Schreibt ein Double Word (32 Bit) an einen I/O Port
 */
static inline void outl(uint16_t port, uint32_t value) {
    __asm__ volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}

/*
 * inl - Liest ein Double Word von einem I/O Port
 */
static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    __asm__ volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/*
 * io_wait - Kurze Verzögerung für I/O Timing
 *
 * Manche Hardware braucht Zeit zwischen I/O Operationen.
 * Ein Dummy-Write an Port 0x80 (POST Code Port) verzögert genau richtig.
 */
static inline void io_wait(void) {
    outb(0x80, 0);
}

#endif /* KIOS_IO_H */
