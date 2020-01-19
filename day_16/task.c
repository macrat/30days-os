#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "desctable.h"
#include "queue.h"
#include "task.h"
#include "timer.h"


#define TASK_SWITCH_INTERVAL  2
#define TASK_SELECTOR_START   3
#define TASK_QUEUE_SIZE       128


typedef struct TaskInfo {
    bool                  active;
    task_address_t        address;
    task_status_segment_t tss;
} task_info_t;

typedef struct TaskManager {
    task_info_t    pool[TASKS_MAX];
    task_t         current;
    queue_t        queue[];
} task_manager_t;

static task_manager_t* _task_manager;


static inline bool is_same_task(task_t a, task_t b) {
    return a.selector == b.selector;
}

void task_switch(task_t task) {
    if (is_same_task(task, _task_manager->current)) return;

    _task_manager->current = task;

    __asm__(
        "LJMP  *%0;"
        :
        : "m"(task)
    );
}

static void task_switch_to_next() {
    task_t next;

    if (queue_pop_and_push(_task_manager->queue, &next)) {
        task_switch(next);
    }
}

static void auto_task_switch(void *ptr) {
    set_timeout(auto_task_switch, ptr, TASK_SWITCH_INTERVAL);

    task_switch_to_next();
}

static void idle_task(void* ptr) {
    (void)ptr;

    while (1) __asm__("HLT");
}

void init_task_manager() {
    _task_manager = malloc(offsetof(task_manager_t, queue) + queue_sizeof(TASK_QUEUE_SIZE, sizeof(task_t)));

    for (int i = 0; i < TASKS_MAX; i++) {
        memset(&_task_manager->pool[i], 0x00, sizeof(task_info_t));

        _task_manager->pool[i].address.offset = 0;
        _task_manager->pool[i].address.selector = (i + TASK_SELECTOR_START) * 8;
    }

    create_queue_on(_task_manager->queue, TASK_QUEUE_SIZE, sizeof(task_t));

    _task_manager->pool[0].active = true;
    _task_manager->pool[0].tss.iomap = 0x40000000;
    set_task_segment(TASK_SELECTOR_START, &_task_manager->pool[0].tss);

    __asm__(
        "LTR  %%AX;"
        :
        : "a"(_task_manager->pool[0].address.selector)
    );
    _task_manager->current = _task_manager->pool[0].address;

    queue_push(_task_manager->queue, &_task_manager->pool[0].address);

    task_start(idle_task, 0);

    set_timeout(auto_task_switch, 0, TASK_SWITCH_INTERVAL);
}

static task_info_t* task_allocate() {
    for (int i = 0; i < TASKS_MAX; i++) {
        if (!_task_manager->pool[i].active) {
            _task_manager->pool[i].active = true;
            return &_task_manager->pool[i];
        }
    }
    return 0;
}

task_t task_start(task_func_t func, void* arg) {
    task_info_t* const task = task_allocate();
    if (task == 0) return (task_t){0, 0};

    task->tss.iomap = 0x40000000;

    task->tss.eip = (uint32_t)func;
    task->tss.eflags = 0x00000202;  // IF = 1;
    task->tss.eax = task->tss.ecx = task->tss.edx = task->tss.ebx = 0;
    task->tss.esp = (uint32_t)(malloc(64 * 1024) + 64 * 1024 - 8);
    *((void**)(task->tss.esp + 4)) = arg;
    task->tss.ebp = task->tss.esi = task->tss.edi = 0;
    task->tss.es = task->tss.ss = task->tss.ds = task->tss.fs = task->tss.gs = 1 * 8;
    task->tss.cs = 2 * 8;

    set_task_segment(task->address.selector/8, &task->tss);

    queue_push(_task_manager->queue, &task->address);

    return (task_t)task->address;
}

bool task_is_running(task_t task) {
    for (const queue_item_t* itr = _task_manager->queue->first; itr; itr = itr->next) {
        if (is_same_task(*(task_t*)itr->payload, task)) {
            return true;
        }
    }
    return false;
}

void task_sleep(task_t task) {
    __asm__("CLI");

    queue_drop_by_data(_task_manager->queue, &task);

    __asm__("STI");
}

void task_wakeup(task_t task) {
    __asm__("CLI");

    if (!task_is_running(task)) {
        queue_push(_task_manager->queue, &task);
    }

    __asm__("STI");

    task_switch(task);
}

int get_running_task_num() {
    return queue_item_count(_task_manager->queue);
}

int get_active_task_num() {
    int count = 0;
    for (int i = 0; i < TASKS_MAX; i++) {
        count += _task_manager->pool[i].active;
    }
    return count;
}

task_t get_current_task() {
    return _task_manager->current;
}
