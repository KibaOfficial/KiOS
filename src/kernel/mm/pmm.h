// Copyright (c) 2026 KibaOfficial
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "types.h"

void pmm_init(void);
void* pmm_alloc_page(void);
void pmm_free_page(void* phys);

uint64_t pmm_total_pages(void);
uint64_t pmm_used_pages(void);