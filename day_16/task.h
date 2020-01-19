#pragma once

#include <stdint.h>

#include "desctable.h"


#define TASKS_MAX             1024


typedef void (*task_func_t)(void*);

typedef struct TaskAddress {
    uint32_t offset;
    uint16_t selector;
} task_address_t;

typedef task_address_t task_t;


extern void init_task_manager();

extern task_t task_start(task_func_t func, void* arg);
extern bool task_is_running(task_t task);
extern void task_switch(task_t task);
extern void task_sleep(task_t task);
extern void task_wakeup(task_t task);

extern int get_running_task_num();
extern int get_active_task_num();

extern task_t get_current_task();
