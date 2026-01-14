// Copyright (c) 2026 KibaOfficial
// TSS (Task State Segment) Initialisierung für x86_64
//
#include "tss.h"
#include <string.h>

// TSS-Instanz (wird im Kernel-BSS angelegt)
tss_t tss __attribute__((aligned(16)));

void tss_init(void* df_stack, uint64_t df_stack_size) {
    memset(&tss, 0, sizeof(tss));
    // Setze IST1 auf das Ende des bereitgestellten Stacks (x86_64 wächst nach unten)
    tss.ist1 = (uint64_t)df_stack + df_stack_size;
    // IO Map Base auf das Ende der TSS setzen (keine IO-Map)
    tss.io_map_base = sizeof(tss_t);
}
