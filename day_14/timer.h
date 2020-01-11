#pragma once

#include <stdint.h>

#include "events.h"


#define SECOND  ((uint32_t)100)
#define MINUTE  (60*SECOND)
#define HOUR    (60*MINUTE)


typedef void (*timeout_handler_t)(void* arg);


extern void init_timeout_manager();
extern void on_timer(const event_t* ev);
extern int set_timeout(timeout_handler_t handler, void* arg, uint32_t timeout);
