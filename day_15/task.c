#include "desctable.h"
#include "memory.h"
#include "task.h"
#include "timer.h"


#define TASK_SWITCH_INTERVAL  2


typedef struct Task {
    struct {
        uint32_t offset;
        uint16_t selector;
    } jump_address;
    task_status_segment_t tss;
} task_t;


static task_t _tasks[2];
static uint32_t _current_task = 0;
static const uint32_t _task_num = 2;


static void auto_task_switch(void *ptr) {
    set_timeout(auto_task_switch, ptr, TASK_SWITCH_INTERVAL);
    task_switch((_current_task + 1) % _task_num);
}

void init_tasks(void (*task_b_main)()) {
    _tasks[0].tss.ldtr  = _tasks[1].tss.ldtr  = 0;
    _tasks[0].tss.iomap = _tasks[1].tss.iomap = 0x40000000;

    _tasks[0].jump_address.offset = _tasks[1].jump_address.offset = 0;

    _tasks[0].jump_address.selector = set_task_segment(&_tasks[0].tss) * 8;
    _tasks[1].jump_address.selector = set_task_segment(&_tasks[1].tss) * 8;

    __asm__(
        "LTR  %%AX;"
        :
        : "a"(_tasks[0].jump_address.selector)
    );

    _tasks[1].tss.eip = (uint32_t)task_b_main;
    _tasks[1].tss.eflags = 0x00000202;  // IF = 1;
    _tasks[1].tss.eax = _tasks[1].tss.ecx = _tasks[1].tss.edx = _tasks[1].tss.ebx = 0;
    _tasks[1].tss.esp = (uint32_t)(malloc(64 * 1024) + 64 * 1024);
    _tasks[1].tss.ebp = _tasks[1].tss.esi = _tasks[1].tss.edi = 0;
    _tasks[1].tss.es = _tasks[1].tss.ss = _tasks[1].tss.ds = _tasks[1].tss.fs = _tasks[1].tss.gs = 1 * 8;
    _tasks[1].tss.cs = 2 * 8;

    set_timeout(auto_task_switch, 0, TASK_SWITCH_INTERVAL);
}

void task_switch(uint32_t id) {
    __asm__(
        "LJMP  *%0;"
        :
        : "m"(_tasks[_current_task = id].jump_address)
    );
}
