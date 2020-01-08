#include <stdint.h>

#include "desctable.h"
#include "events.h"
#include "graphics.h"
#include "hankaku.h"
#include "hello.h"
#include "interrupt.h"


bootinfo_t *get_bootinfo() {
    return (bootinfo_t*)(0x0ff0);
}

void wait_keyboard_controller_ready() {
    while (io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY);
}

void init_keyboard() {
    wait_keyboard_controller_ready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_keyboard_controller_ready();
    io_out8(PORT_KEYDAT, KBC_MODE);
}

void enable_mouse() {
    wait_keyboard_controller_ready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
    wait_keyboard_controller_ready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
}

void on_keyboard(uint8_t data) {
    static int receive_count = 0;
    receive_count++;

    draw_rect(0, 0, 8*10, 16*2, COLOR_BLACK);
    draw_int(0, 0, data, 16, get_hankaku_font(), COLOR_LIGHT_GRAY);
    draw_int(0, 16, receive_count, 10, get_hankaku_font(), COLOR_LIGHT_GRAY);
}

void on_mouse(uint8_t data) {
    static int receive_count = 0;
    receive_count++;

    draw_rect(8*11, 0, 8*10, 16*2, COLOR_BLACK);
    draw_int(8*11, 0, data, 16, get_hankaku_font(), COLOR_LIGHT_GRAY);
    draw_int(8*11, 16, receive_count, 10, get_hankaku_font(), COLOR_LIGHT_GRAY);
}

void main() {
    init_descriptor_table();
    init_pic();
    io_sti();
    init_keyboard();
    enable_mouse();

    set_palette(SIMPLE_COLORS);

    draw_rect(0, 0, get_screen_x(), get_screen_y(), COLOR_WHITE);
    draw_rect(20, 20, 30, 30, COLOR_RED);

    draw_text(55, 20, "012345678901234567890\nhello\tworld!\nthis\tis\ta\ttest", get_hankaku_font(), COLOR_RED);

    draw_mouse_cursor(20, 60);

    enable_interrupt();

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
            on_keyboard(ev->data);
            break;
        case EVENT_MOUSE:
            on_mouse(ev->data);
            break;
        }
    }
}
