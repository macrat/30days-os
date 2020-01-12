#pragma once

#include <stdint.h>


extern void init_tasks(void (*task_b_main)());
extern void task_switch(uint32_t id);
