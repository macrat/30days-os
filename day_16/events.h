#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "task.h"


typedef enum EventType {
    EVENT_RAW_KEYBOARD = 1,
    EVENT_RAW_MOUSE,
    EVENT_KEYBOARD,
    EVENT_MOUSE,
} event_type_t;

typedef struct EventData {
    uint32_t a, b, c, d;
} event_data_t;

typedef struct Event {
    event_type_t type;
    union {
        uint8_t      uint8[16];
        uint16_t     uint16[8];
        uint32_t     uint32[4];
        event_data_t data;
    };
} event_t;


extern void init_event_queue();
extern const event_t* pop_event(event_t* buf);
extern void push_event(event_t);

extern void observe_event(task_t task);
