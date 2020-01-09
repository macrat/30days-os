#include <stdbool.h>

#include "hello.h"
#include "memory.h"


#define EFLAGS_AC_BIT     0x00040000
#define CR0_CACHE_DISABLE 0x60000000

#define MEMORY_MANAGER_MAX_FREES  4096


struct FreeMemoryInfo {
    void*  address;
    size_t size;
} typedef free_memory_info_t;

struct MemoryManager {
    uint32_t           count;
    free_memory_info_t frees[MEMORY_MANAGER_MAX_FREES];
} typedef memory_manager_t;

static memory_manager_t* const memory_manager = (memory_manager_t*)0x3c0000;


bool is_cpu_i486() {
    const uint32_t eflag = io_load_eflags();

    io_store_eflags(eflag | EFLAGS_AC_BIT);

    const bool is486 = io_load_eflags() & EFLAGS_AC_BIT;

    io_store_eflags(eflag);

    return is486;
}

uint32_t load_cr0() {
    uint32_t cr0 = 0;
    asm(
        "MOV  %%EAX, %%CR0;"
        :
        : "a"(cr0)
    );
    return cr0;
}

void store_cr0(uint32_t cr0) {
    asm(
        "MOV  %%CR0, %%EAX;"
        : "=a"(cr0)
    );
}

void disable_memory_cache() {
    store_cr0(load_cr0() | CR0_CACHE_DISABLE);
}

void enable_memory_cache() {
    store_cr0(load_cr0() & ~CR0_CACHE_DISABLE);
}

size_t scan_memory_size(const void* start, const void* end) {
    const bool is486 = is_cpu_i486();

    if (is486) disable_memory_cache();

    const void* ptr = start;
    while (ptr <= end && is_available_address(ptr + 0xffc)) {
        ptr += 0x1000;
    }

    if (is486) enable_memory_cache();

    return (size_t)ptr;
}

void init_memory_manager() {
    memory_manager->count = 0;
}

size_t free_memory_total() {
    size_t total;
    for (uint32_t i = 0; i < memory_manager->count; i++) {
        total += memory_manager->frees[i].size;
    }
    return total;
}

void drop_memory_manager_entry(uint32_t index) {
    memory_manager->count--;
    for (uint32_t i = index; i < memory_manager->count; i++) {
        memory_manager->frees[i] = memory_manager->frees[i + 1];
    }
}

void* malloc(size_t size) {
    for (uint32_t i = 0; i < memory_manager->count; i++) {
        free_memory_info_t* free = &memory_manager->frees[i];

        if (free->size < size) continue;

        void* const allocated = free->address;

        free->address += size;
        free->size -= size;

        if (free->size == 0) {
            drop_memory_manager_entry(i);
        }

        return allocated;
    }

    return 0;
}

uint32_t memory_manager_search_index(void* address) {
    uint32_t i = 0;

    for (; i < memory_manager->count && memory_manager->frees[i].address < address; i++);

    return i;
}

int register_memory_area(void* address, size_t size) {
    const uint32_t idx = memory_manager_search_index(address);
    free_memory_info_t* const frees = memory_manager->frees;

    if (idx > 0 && frees[idx - 1].address + frees[idx - 1].size == address) {
        frees[idx - 1].size += size;

        if (idx < memory_manager->count && address + size == frees[idx].address) {
            frees[idx - 1].size += frees[idx].size;
            drop_memory_manager_entry(idx);
        }

        return 0;
    }

    if (idx < memory_manager->count && address + size == frees[idx].address) {
        frees[idx].address = address;
        frees[idx].size += size;
        return 0;
    }

    if (memory_manager->count >= MEMORY_MANAGER_MAX_FREES) return 1;

    for (uint32_t i = memory_manager->count; i > idx; i--) {
        frees[i] = frees[i - 1];
    }
    memory_manager->count++;
    frees[idx].address = address;
    frees[idx].size = size;

    return 0;
}
