#pragma once

#include <stdint.h>


typedef struct TaskStatusSegment {
    uint32_t backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
    uint32_t eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldtr, iomap;
} task_status_segment_t;

extern void init_descriptor_table();

extern uint32_t set_task_segment(task_status_segment_t* tss);
