#include <stdint.h>

#include "desctable.h"
#include "events.h"
#include "graphics.h"
#include "hankaku.h"
#include "hello.h"
#include "interrupt.h"
#include "math.h"


bootinfo_t *get_bootinfo() {
    return (bootinfo_t*)(0x0ff0);
}

struct MouseState {
    uint8_t buf[3];
    int8_t  index;
    uint32_t x, y;
    uint8_t buttons;
} typedef mouse_state_t;

void wait_keyboard_controller_ready() {
    while (io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY);
}

void init_keyboard() {
    wait_keyboard_controller_ready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_keyboard_controller_ready();
    io_out8(PORT_KEYDAT, KBC_MODE);
}

void init_mouse(mouse_state_t *state) {
    wait_keyboard_controller_ready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
    wait_keyboard_controller_ready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);

    state->index = -1;
    state->x = get_screen_x() / 2;
    state->y = get_screen_y() / 2;
}

void draw_screen(mouse_state_t *mouse) {
    draw_rect(0, 0, get_screen_x(), get_screen_y(), COLOR_WHITE);
    draw_rect(20, 20, 30, 30, COLOR_RED);

    draw_text(55, 20, "012345678901234567890\nhello\tworld!\nthis\tis\ta\ttest", get_hankaku_font(), COLOR_RED);

    draw_mouse_cursor(mouse->x, mouse->y);
}

void on_keyboard(event_t *ev) {
    static int receive_count = 0;
    receive_count++;

    draw_rect(0, 0, 8*10, 16*2, COLOR_BLACK);
    draw_int(0, 0, ev->data, 16, get_hankaku_font(), COLOR_LIGHT_GRAY);
    draw_int(0, 16, receive_count, 10, get_hankaku_font(), COLOR_LIGHT_GRAY);
}

void on_raw_mouse(mouse_state_t *state, event_t *ev) {
    if (state->index < 0) {
        if (ev->data == 0xfa) {
            state->index = 0;
        }

        return;
    }

    if (state->index == 0 && ev->data & 0xc8 != 0x08) return;  // drop invalid data

    state->buf[state->index] = ev->data;
    state->index++;

    if (state->index >= 3) {
        event_t ev = {
            .type = EVENT_MOUSE,
            .mouse = {
                .buttons = state->buf[0] & 0x07,
                .x = state->buf[1],
                .y = state->buf[2],
            },
        };

        if (state->buf[0] & 0x10) {
            ev.mouse.x |= 0xffffff00;
        }
        if (state->buf[0] & 0x20) {
            ev.mouse.x |= 0xffffff00;
        }
        ev.mouse.y = -ev.mouse.y;

        push_event(ev);

        state->index = 0;
    }
}

void on_mouse(mouse_state_t *state, event_t *ev) {
    static int receive_count = 0;
    receive_count++;

    state->x = max(0, min(get_screen_x(), (int32_t)(state->x + ev->mouse.x)));
    state->y = max(0, min(get_screen_y(), (int32_t)(state->y + ev->mouse.y)));
    state->buttons = ev->mouse.buttons;

    draw_screen(state);

    draw_rect(8*11, 0, 8*10, 16*2, COLOR_BLACK);
    draw_int(8*11 + 8*0, 0, ev->mouse.x, 16, get_hankaku_font(), COLOR_LIGHT_GRAY);
    draw_int(8*11 + 8*3, 0, ev->mouse.y, 16, get_hankaku_font(), COLOR_LIGHT_GRAY);
    draw_int(8*11 + 8*6, 0, ev->mouse.buttons, 16, get_hankaku_font(), COLOR_LIGHT_GRAY);
    draw_int(8*11, 16, receive_count, 10, get_hankaku_font(), COLOR_LIGHT_GRAY);
}

void main() {
    mouse_state_t mouse_state;

    init_descriptor_table();
    init_pic();
    io_sti();
    enable_interrupt();
    init_keyboard();
    init_mouse(&mouse_state);

    set_palette(SIMPLE_COLORS);

    draw_screen(&mouse_state);

    while (1) {
        io_cli();
        event_t* ev = pop_event();
        io_sti();

        if (ev == 0) {
            io_hlt();
            continue;
        }

        switch (ev->type) {
        case EVENT_KEYBOARD:
            on_keyboard(ev);
            break;
        case EVENT_RAW_MOUSE:
            on_raw_mouse(&mouse_state, ev);
            break;
        case EVENT_MOUSE:
            on_mouse(&mouse_state, ev);
            break;
        }
    }
}
