#include "hello.h"
#include "graphics.h"
#include "math.h"
#include "hankaku.h"
#include "input_devices.h"


struct MouseDecoder {
    uint8_t       buf[3];
    int8_t        index;
    mouse_state_t decoded;
} typedef mouse_decoder_t;

static mouse_decoder_t mouse_decoder = {0};


void wait_keyboard_controller_ready() {
    while (io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY);
}

void init_keyboard() {
    wait_keyboard_controller_ready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_keyboard_controller_ready();
    io_out8(PORT_KEYDAT, KBC_MODE);
}

void init_mouse() {
    wait_keyboard_controller_ready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
    wait_keyboard_controller_ready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);

    mouse_decoder.index = -1;
    mouse_decoder.decoded.x = get_screen_x() / 2;
    mouse_decoder.decoded.y = get_screen_y() / 2;
}

void init_input_devices() {
    init_keyboard();
    init_mouse();
}

void on_raw_keyboard(const event_t* ev) {
    event_t ev2 = {
        .type = EVENT_KEYBOARD,
        .data = ev->data,  // TODO: decode keycode here
    };

    push_event(ev2);
}

void decode_mouse() {
    mouse_decoder.decoded.buttons = mouse_decoder.buf[0] & 0x07;

    int32_t mx = mouse_decoder.buf[1];
    int32_t my = mouse_decoder.buf[2];

    if (mouse_decoder.buf[0] & 0x10) {
        mx |= 0xffffff00;
    }
    if (mouse_decoder.buf[0] & 0x20) {
        my |= 0xffffff00;
    }

    mouse_decoder.decoded.x = max(0, min(get_screen_x(), (int32_t)(mouse_decoder.decoded.x + mx)));
    mouse_decoder.decoded.y = max(0, min(get_screen_y(), (int32_t)(mouse_decoder.decoded.y - my)));
}

void on_raw_mouse(const event_t* ev) {
    if (mouse_decoder.index < 0) {
        if (ev->data == 0xfa) {
            mouse_decoder.index = 0;
        }

        return;
    }

    if (mouse_decoder.index == 0 && (ev->data & 0xc8) != 0x08) return;  // drop invalid data

    mouse_decoder.buf[mouse_decoder.index] = ev->data;
    mouse_decoder.index++;

    if (mouse_decoder.index >= 3) {
        decode_mouse();
        mouse_decoder.index = 0;

        event_t ev = {
            .type = EVENT_MOUSE,
            .mouse = mouse_decoder.decoded,
        };
        push_event(ev);
    }
}

const mouse_state_t* get_mouse_state() {
    return &mouse_decoder.decoded;
}
