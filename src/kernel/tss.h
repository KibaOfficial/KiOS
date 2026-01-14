// Copyright (c) 2026 KibaOfficial
// TSS (Task State Segment) für x86_64
//
// Siehe Intel SDM Vol. 3, Kapitel 7.7

#ifndef KIOS_TSS_H
#define KIOS_TSS_H

#include <stdint.h>

// TSS-Struktur für x86_64 (104 Bytes)
typedef struct {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t io_map_base;
} __attribute__((packed)) tss_t;


extern tss_t tss;
void tss_init(void* df_stack, uint64_t df_stack_size);

#endif // KIOS_TSS_H
