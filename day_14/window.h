#pragma once

#include <stdbool.h>

#include "graphics.h"
#include "input_devices.h"


#define WINROD_BORDER_TICKNESS  2

typedef struct Window {
    bool     active;
    sprite_t *fg, *bg;
    void*    extra;
    void     (*on_keyboard)(const keyboard_event_t* ev, struct Window* window);
} window_t;


extern void init_window_manager();
extern window_t* create_window(uint32_t width, uint32_t height);
extern void destroy_window(window_t* window);

extern void focus_window(window_t* window);
extern void move_window(window_t* window, int32_t x, int32_t y);
extern void clear_window(window_t* window);

extern void on_keyboard(const event_t* ev);
extern void on_mouse(const event_t* ev);
