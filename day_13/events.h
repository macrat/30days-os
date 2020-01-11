#pragma once

#include <stdbool.h>
#include <stdint.h>


typedef enum EventType {
    EVENT_RAW_KEYBOARD = 1,
    EVENT_RAW_MOUSE,
    EVENT_KEYBOARD,
    EVENT_MOUSE,
    EVENT_TIMER,
} event_type_t;

typedef struct MouseState {
    uint32_t x, y;
    uint8_t  buttons;
} mouse_state_t;

typedef struct Event {
    bool         active;
    event_type_t type;
    union {
        uint8_t       data;
        uint32_t      timer;
        mouse_state_t mouse;
    };
} event_t;

extern void init_event_queue();
extern const event_t* peek_event();
extern const event_t* pop_event();
extern void push_event(event_t);
