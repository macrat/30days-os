#include <stdbool.h>

#include "hello.h"
#include "memory.h"


#define EFLAGS_AC_BIT     0x00040000
#define CR0_CACHE_DISABLE 0x60000000

#define MEMORY_MANAGER_MAX_FREES  4096
#define MEMORY_MANAGER_MAX_USES   4096


typedef struct FreeMemoryInfo {
    void*  address;
    size_t size;
} free_memory_info_t;

typedef struct UsesMemoryInfo {
    void*  address;
    size_t size;
} uses_memory_info_t;

typedef struct MemoryManager {
    uint32_t           frees_count;
    free_memory_info_t frees[MEMORY_MANAGER_MAX_FREES];
    uint32_t           uses_count;
    uses_memory_info_t uses[MEMORY_MANAGER_MAX_USES];
} memory_manager_t;

static memory_manager_t* const memory_manager = (memory_manager_t*)0x3c0000;


static bool is_cpu_i486() {
    const uint32_t eflag = io_load_eflags();

    io_store_eflags(eflag | EFLAGS_AC_BIT);

    const bool is486 = io_load_eflags() & EFLAGS_AC_BIT;

    io_store_eflags(eflag);

    return is486;
}

static uint32_t load_cr0() {
    uint32_t cr0 = 0;
    __asm__(
        "MOV  %%EAX, %%CR0;"
        :
        : "a"(cr0)
    );
    return cr0;
}

static void store_cr0(uint32_t cr0) {
    __asm__(
        "MOV  %%CR0, %%EAX;"
        : "=a"(cr0)
    );
}

static void disable_memory_cache() {
    store_cr0(load_cr0() | CR0_CACHE_DISABLE);
}

static void enable_memory_cache() {
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
    memory_manager->frees_count = 0;
    memory_manager->uses_count = 0;
}

size_t free_memory_total() {
    size_t total;
    for (uint32_t i = 0; i < memory_manager->frees_count; i++) {
        total += memory_manager->frees[i].size;
    }
    return total;
}

static void drop_memory_manager_entry(uint32_t index) {
    memory_manager->frees_count--;
    for (uint32_t i = index; i < memory_manager->frees_count; i++) {
        memory_manager->frees[i] = memory_manager->frees[i + 1];
    }
}

void* allocate_memory(size_t size) {
    if (memory_manager->uses_count + 1 >= MEMORY_MANAGER_MAX_USES) return 0;

    for (uint32_t i = 0; i < memory_manager->frees_count; i++) {
        free_memory_info_t* free = &memory_manager->frees[i];

        if (free->size < size) continue;

        void* const address = free->address;

        free->address += size;
        free->size -= size;

        if (free->size == 0) {
            drop_memory_manager_entry(i);
        }

        return address;
    }

    return 0;
}

static uint32_t memory_manager_search_index(void* address) {
    uint32_t i = 0;

    for (; i < memory_manager->frees_count && memory_manager->frees[i].address < address; i++);

    return i;
}

int register_memory_area(void* address, size_t size) {
    const uint32_t idx = memory_manager_search_index(address);
    free_memory_info_t* const frees = memory_manager->frees;

    if (idx > 0 && frees[idx - 1].address + frees[idx - 1].size == address) {
        frees[idx - 1].size += size;

        if (idx < memory_manager->frees_count && address + size == frees[idx].address) {
            frees[idx - 1].size += frees[idx].size;
            drop_memory_manager_entry(idx);
        }

        return 0;
    }

    if (idx < memory_manager->frees_count && address + size == frees[idx].address) {
        frees[idx].address = address;
        frees[idx].size += size;
        return 0;
    }

    if (memory_manager->frees_count >= MEMORY_MANAGER_MAX_FREES) return 1;

    for (uint32_t i = memory_manager->frees_count; i > idx; i--) {
        frees[i] = frees[i - 1];
    }
    memory_manager->frees_count++;
    frees[idx].address = address;
    frees[idx].size = size;

    return 0;
}

void* malloc(size_t size) {
    size = (size + 0xfff) & 0xfffff000;

    void* addr = allocate_memory(size);

    if (addr == 0) return 0;

    memory_manager->uses[memory_manager->uses_count].address = addr;
    memory_manager->uses[memory_manager->uses_count].size = size;
    memory_manager->uses_count++;

    return addr;
}

void free(void* address) {
    uint32_t i = 0;
    for (; i < memory_manager->uses_count; i++) {
        if (memory_manager->uses[i].address == address) {
            register_memory_area(address, memory_manager->uses[i].size);
            memory_manager->uses_count--;
            break;
        }
    }
    for (; i < memory_manager->uses_count; i++) {
        memory_manager->uses[i] = memory_manager->uses[i + 1];
    }
}

void* memset(void* dest, int value, size_t len) {
    for (void* itr=dest; itr < dest + len; itr++) {
        *(int8_t*)itr = (int8_t)value;
    }
    return dest;
}

void* memcpy(void* dest, const void* src, size_t len) {
    for (uint8_t *d=(uint8_t*)dest, *s=(uint8_t*)src; d < (uint8_t*)dest + len; d++, s++) {
        *d = *s;
    }
    return dest;
}
