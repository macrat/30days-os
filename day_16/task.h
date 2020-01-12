#pragma once

#include <stdint.h>

#include "desctable.h"


#define TASKS_MAX             1000


typedef struct Task {
    struct {
        uint32_t offset;
        uint16_t selector;
    } jump_address;

    bool                  active;
    task_status_segment_t tss;
    struct Task           *next, *prev;
} task_t;

typedef void (*task_func_t)(void*);


extern void init_task_manager();

extern task_t* task_start(task_func_t func, void* arg);
extern void task_switch(task_t* task);
extern void task_sleep(task_t* task);
extern void task_wakeup(task_t* task);

extern task_t* get_current_task();
extern int get_running_task_num();
extern int get_active_task_num();
