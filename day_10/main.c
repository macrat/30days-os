#include <stdint.h>

#include "desctable.h"
#include "events.h"
#include "graphics.h"
#include "hankaku.h"
#include "hello.h"
#include "input_devices.h"
#include "interrupt.h"
#include "math.h"
#include "memory.h"


uint32_t io_load_eflags() {
    uint32_t eax = 0;
    asm(
        "PUSHFL;"
        "POP  %%EAX;"
        :
        : "a"(eax)
    );
    return eax;
}

void io_store_eflags(uint32_t eflags) {
    asm(
        "PUSH  %%EAX;"
        "POPFL;"
        : "=a"(eflags)
    );
}

sprite_t* create_desktop() {
    sprite_t* desktop = create_sprite(0, 0, get_screen_width(), get_screen_height(), 0);

    draw_rect(desktop, 0, 0, get_screen_width(), get_screen_height(), COLOR_WHITE);
    draw_rect(desktop, 20, 20, 30, 30, COLOR_RED);

    draw_text(desktop, 55, 20, "012345678901234567890\nhello\tworld!\nthis\tis\ta\ttest", get_hankaku_font(), COLOR_RED);

    return desktop;
}

void on_keyboard(const event_t* ev, sprite_t* card) {
    static int receive_count = 0;
    receive_count++;

    draw_rect(card, 0, 0, 8*10, 16*2, COLOR_LIGHT_GRAY);
    draw_int(card, 0, 0, ev->data, 16, get_hankaku_font(), COLOR_DARK_GRAY);
    draw_int(card, 0, 16, receive_count, 10, get_hankaku_font(), COLOR_DARK_GRAY);
}

void on_mouse(const event_t* ev, sprite_t* cursor) {
    move_cursor_sprite(cursor, &ev->mouse);
}

void main() {
    init_descriptor_table();
    init_pic();
    asm("STI");

    init_memory_manager();
    register_memory_area((void*)0x00001000, 0x0009e000);
    register_memory_area((void*)0x00400000, scan_memory_size((void*)0x00400000, (void*)0xbfffffff) - 0x00400000);

    init_event_queue();
    enable_interrupt();
    init_input_devices();
    init_screen();

    create_desktop();
    sprite_t* cursor = create_cursor_sprite();
    sprite_t* keycard = create_sprite(10, get_screen_height()-16*2-10, 8*10, 16*2, 10);
    refresh_screen();

    set_palette(SIMPLE_COLORS);

    while (1) {
        asm("CLI");
        const event_t* const ev = pop_event();
        asm("STI");

        if (ev == 0) {
            refresh_screen();
            asm("HLT");
            continue;
        }

        switch (ev->type) {
        case EVENT_RAW_KEYBOARD:
            on_raw_keyboard(ev);
            break;
        case EVENT_RAW_MOUSE:
            on_raw_mouse(ev);
            break;
        case EVENT_KEYBOARD:
            on_keyboard(ev, keycard);
            break;
        case EVENT_MOUSE:
            on_mouse(ev, cursor);
            break;
        }
    }
}
