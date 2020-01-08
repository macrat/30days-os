#pragma once

#include <stdint.h>


typedef uint16_t event_type_t;

#define EVENT_KEYBOARD   ((event_type_t) 0x0021)
#define EVENT_RAW_MOUSE  ((event_type_t) 0x002c)
#define EVENT_MOUSE      ((event_type_t) 0x012c)

struct Event {
    uint8_t      proceed;
    event_type_t type;
    union {
        uint8_t data;
        struct mouse {
            int8_t x, y;
            uint8_t buttons;
        } mouse;
    };
} typedef event_t;

#define EVENT_QUEUE_SIZE  512

struct EventQueue {
    event_t queue[EVENT_QUEUE_SIZE];
    uint_fast16_t read_index, write_index;
} typedef event_queue_t;

extern event_t* peek_event();
extern event_t* pop_event();
extern void push_event(event_t);
