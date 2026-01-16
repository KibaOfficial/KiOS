// Copyright (c) 2026 KibaOfficial
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "types.h"

#define MEMORY_MAP_BASE 0x10000

typedef struct {
  uint64_t base;
  uint64_t length;
  uint32_t type;
  uint32_t reserved;
} __attribute__((packed)) memory_map_entry_t;

static inline uint16_t memory_map_entry_count() {
  return *(uint16_t*)MEMORY_MAP_BASE;
}

static inline memory_map_entry_t* memory_map_entries() {
  return (memory_map_entry_t*)(MEMORY_MAP_BASE + 2);
}
