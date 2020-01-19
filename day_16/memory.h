#pragma once

#include <stddef.h>
#include <stdint.h>


extern size_t scan_memory_size(const void* start, const void* end);

extern void init_memory_manager();
extern size_t free_memory_total();
extern int register_memory_area(void* address, size_t size);
