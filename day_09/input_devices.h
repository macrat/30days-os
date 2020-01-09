#pragma once

#include "events.h"


extern void init_input_devices();

extern void on_raw_keyboard(const event_t* ev);
extern void on_raw_mouse(const event_t* ev);

const mouse_state_t* get_mouse_state();
