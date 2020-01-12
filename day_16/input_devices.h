#pragma once

#include "events.h"


#define MOUSE_LEFT    ((uint8_t) 0x01)
#define MOUSE_RIGHT   ((uint8_t) 0x02)
#define MOUSE_CENTER  ((uint8_t) 0x04)

typedef struct MouseState {
    int32_t x, y;
    uint8_t buttons;
} mouse_state_t;

#define KEY_MOD_CTRL         ((uint8_t) 0x01)
#define KEY_MOD_LEFT_SHIFT   ((uint8_t) 0x02)
#define KEY_MOD_RIGHT_SHIFT  ((uint8_t) 0x04)
#define KEY_MOD_SHIFT        (KEY_MOD_LEFT_SHIFT | KEY_MOD_RIGHT_SHIFT)
#define KEY_MOD_ALT          ((uint8_t) 0x08)

typedef enum KeyEventType {
    KEY_PRESS,
    KEY_RELEASE,
} key_event_type_t;

typedef struct KeyboardEvent {
    key_event_type_t type;
    uint8_t          character;
    uint8_t          keycode;
    uint8_t          modifier;
} keyboard_event_t;


extern void init_input_devices();

extern void on_raw_keyboard(const event_t* ev);
extern void on_raw_mouse(const event_t* ev);

extern const mouse_state_t* get_mouse_state();
