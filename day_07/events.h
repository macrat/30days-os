#pragma once

#include <stdint.h>


typedef uint8_t event_type_t;

#define EVENT_KEYBOARD  ((event_type_t) 0x21)
#define EVENT_MOUSE     ((event_type_t) 0x2c)

struct Event {
    uint8_t      active;
    event_type_t type;
    uint8_t      data;
} typedef event_t;

#define EVENT_QUEUE_SIZE  512

struct EventQueue {
    event_t queue[EVENT_QUEUE_SIZE];
    uint_fast16_t read_index;
    uint_fast16_t write_index;
} typedef event_queue_t;

extern event_t* peek_event();
extern event_t* pop_event();
extern void push_event(event_t);
