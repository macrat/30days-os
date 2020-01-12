#include <stdbool.h>

#include "desctable.h"
#include "memory.h"
#include "task.h"
#include "timer.h"


#define TASK_SWITCH_INTERVAL  2
#define TASK_SELECTOR_START   3


typedef struct TaskManager {
    task_t* current;
    task_t  pool[TASKS_MAX];
} task_manager_t;

static task_manager_t* _task_manager;


void task_switch(task_t* task) {
    if (task == _task_manager->current) return;

    _task_manager->current = task;

    __asm__(
        "LJMP  *%0;"
        :
        : "m"(task->jump_address)
    );
}

static void auto_task_switch(void *ptr) {
    set_timeout(auto_task_switch, ptr, TASK_SWITCH_INTERVAL);
    task_switch(_task_manager->current->next);
}

static void idle_task(void* ptr) {
    (void)ptr;

    while (1) __asm__("HLT");
}

void init_task_manager() {
    _task_manager = malloc(sizeof(task_manager_t));
    memset(_task_manager, 0x00, sizeof(task_manager_t));

    for (int i = 0; i < TASKS_MAX; i++) {
        _task_manager->pool[i].jump_address.selector = (i + TASK_SELECTOR_START) * 8;
    }

    task_t* cur = _task_manager->current = &_task_manager->pool[0];
    cur->next = cur->prev = cur;
    cur->active = true;

    cur->tss.iomap = 0x40000000;

    set_task_segment(TASK_SELECTOR_START, &cur->tss);

    __asm__(
        "LTR  %%AX;"
        :
        : "a"(cur->jump_address.selector)
    );

    auto_task_switch(0);

    task_start(idle_task, 0);
}

static task_t* task_allocate() {
    for (int i = 0; i < TASKS_MAX; i++) {
        if (!_task_manager->pool[i].active) {
            _task_manager->pool[i].active = true;
            return &_task_manager->pool[i];
        }
    }
    return 0;
}

task_t* task_start(task_func_t func, void* arg) {
    task_t* const task = task_allocate();
    if (task == 0) return 0;

    task->tss.iomap = 0x40000000;

    task->tss.eip = (uint32_t)func;
    task->tss.eflags = 0x00000202;  // IF = 1;
    task->tss.eax = task->tss.ecx = task->tss.edx = task->tss.ebx = 0;
    task->tss.esp = (uint32_t)(malloc(64 * 1024) + 64 * 1024 - 8);
    *((void**)(task->tss.esp + 4)) = arg;
    task->tss.ebp = task->tss.esi = task->tss.edi = 0;
    task->tss.es = task->tss.ss = task->tss.ds = task->tss.fs = task->tss.gs = 1 * 8;
    task->tss.cs = 2 * 8;

    set_task_segment(task->jump_address.selector/8, &task->tss);

    task_wakeup(task);

    return task;
}

void task_sleep(task_t* task) {
    if (task == 0) {
        task = _task_manager->current;
    }

    if (!task->active) return;

    __asm__("CLI");

    task->prev->next = task->next;
    task->next->prev = task->prev;

    __asm__("STI");

    if (task == _task_manager->current) {
        task_switch(task->next);
    }
}

void task_wakeup(task_t* task) {
    if (!task->active) return;

    __asm__("CLI");

    task->prev = _task_manager->current;
    task->next = _task_manager->current->next;

    task->prev->next = task;
    task->next->prev = task;

    __asm__("STI");

    task_switch(task);
}

task_t* get_current_task() {
    return _task_manager->current;
}

int get_running_task_num() {
    int count = 1;

    for (task_t* itr = _task_manager->current; itr->next != _task_manager->current; itr = itr->next) {
        count++;
    }

    return count;
}

int get_active_task_num() {
    int count = 0;
    for (int i = 0; i < TASKS_MAX; i++) {
        count += _task_manager->pool[i].active;
    }
    return count;
}
