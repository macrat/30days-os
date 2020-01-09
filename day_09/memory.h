#pragma once

#include <stddef.h>
#include <stdint.h>


size_t scan_memory_size(const void* start, const void* end);

void init_memory_manager();
size_t free_memory_total();
void* malloc(size_t size);
int register_memory_area(void* address, size_t size);
