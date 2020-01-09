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

bootinfo_t* get_bootinfo() {
    return (bootinfo_t*)(0x0ff0);
}

void draw_screen() {
    draw_rect(0, 0, get_screen_x(), get_screen_y(), COLOR_WHITE);
    draw_rect(20, 20, 30, 30, COLOR_RED);

    draw_text(55, 20, "012345678901234567890\nhello\tworld!\nthis\tis\ta\ttest", get_hankaku_font(), COLOR_RED);

    draw_mouse_cursor(get_mouse_state());
}

void on_keyboard(const event_t* ev) {
    static int receive_count = 0;
    receive_count++;

    draw_rect(0, 0, 8*10, 16*2, COLOR_BLACK);
    draw_int(0, 0, ev->data, 16, get_hankaku_font(), COLOR_LIGHT_GRAY);
    draw_int(0, 16, receive_count, 10, get_hankaku_font(), COLOR_LIGHT_GRAY);
}

void on_mouse(const event_t* ev) {
    draw_screen();
}

void main() {
    init_descriptor_table();
    init_pic();
    asm("STI");
    enable_interrupt();
    init_input_devices();

    const uint32_t memtotal = scan_memory_size((void*)0x00400000, (void*)0xbfffffff);
    init_memory_manager();
    register_memory_area((void*)0x00001000, 0x0009e000);
    register_memory_area((void*)0x00400000, memtotal - 0x00400000);

    set_palette(SIMPLE_COLORS);

    draw_screen();

    draw_rect(0, get_screen_y()-32, get_screen_x(), 32, COLOR_BLACK);
    draw_text(0, get_screen_y()-32, "memory (MB):", get_hankaku_font(), COLOR_WHITE);
    draw_int(8*13, get_screen_y()-32, memtotal / 1024 / 1024, 10, get_hankaku_font(), COLOR_WHITE);
    draw_text(0, get_screen_y()-16, "free   (KB):", get_hankaku_font(), COLOR_WHITE);
    draw_int(8*13, get_screen_y()-16, free_memory_total() / 1024, 10, get_hankaku_font(), COLOR_WHITE);

    while (1) {
        asm("CLI");
        const event_t* const ev = pop_event();
        asm("STI");

        if (ev == 0) {
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
            on_keyboard(ev);
            break;
        case EVENT_MOUSE:
            on_mouse(ev);
            break;
        }
    }
}
