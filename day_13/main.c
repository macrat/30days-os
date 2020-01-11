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
#include "strings.h"
#include "timer.h"


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

#define BENCHMARK_NUM      6
#define BENCHMARK_DURATION 10

typedef struct Benchmark {
    sprite_t* viewer;
    uint32_t  counter;
    uint32_t  history[BENCHMARK_NUM];
    int8_t    index;
} benchmark_t;

void show_benchmark(void* ptr) {
    benchmark_t* bm = (benchmark_t*)ptr;
    if (bm->index >= BENCHMARK_NUM) {
        return;
    }
    if (bm->index >= 0) {
        bm->history[bm->index] = bm->counter;

        fill_sprite(bm->viewer, COLOR_WHITE);

        for (int line = 0; line <= bm->index; line++) {
            char buf[1024];
            size_t len = strlen(itoa_s(bm->history[line] / BENCHMARK_DURATION, buf, sizeof(buf), 10));
            strcpy_s(buf + len, sizeof(buf) - len, "ops/sec");

            draw_text(bm->viewer, 0, 16*line, buf, get_hankaku_font(), COLOR_DARK_GRAY);
        }
    }

    bm->index++;

    bm->counter = 0;
    set_timeout(show_benchmark, ptr, BENCHMARK_DURATION*SECOND);
}

void dummy_timeout(void* ptr) {
    set_timeout(dummy_timeout, ptr, (uint32_t)ptr);
}

void main() {
    init_descriptor_table();
    init_pic();
    asm("STI");

    init_memory_manager();
    register_memory_area((void*)0x00001000, 0x0009e000);
    register_memory_area((void*)0x00400000, scan_memory_size((void*)0x00400000, (void*)0xbfffffff) - 0x00400000);

    init_event_queue();
    init_pit();
    enable_interrupt();
    init_input_devices();
    init_timeout_manager();
    init_screen();

    create_desktop();
    sprite_t* cursor = create_cursor_sprite();
    sprite_t* keycard = create_sprite(10, get_screen_height()-16*2-10, 8*10, 16*2, 10);
    refresh_screen();

    set_palette(SIMPLE_COLORS);

    benchmark_t bm = {0};
    bm.viewer = create_sprite(get_screen_width()/2 - 8*16/2, 100, 8*16, 16*BENCHMARK_NUM, 200);
    bm.index = -1;  // drop first
    set_timeout(show_benchmark, (void*)&bm, 3*SECOND);
    set_timeout(dummy_timeout, (void*)(1*SECOND), 1*SECOND);
    set_timeout(dummy_timeout, (void*)300, 300);
    set_timeout(dummy_timeout, (void*)50, 50);

    while (1) {
        bm.counter++;

        asm("CLI");
        const event_t* const evp = pop_event();

        if (evp == 0) {
            refresh_screen();
            asm(
                "STI;"
                //"HLT;"
            );
            continue;
        }
        const event_t ev = *evp;
        asm("STI");

        switch (ev.type) {
        case EVENT_RAW_KEYBOARD:
            on_raw_keyboard(&ev);
            break;
        case EVENT_RAW_MOUSE:
            on_raw_mouse(&ev);
            break;
        case EVENT_KEYBOARD:
            on_keyboard(&ev, keycard);
            break;
        case EVENT_MOUSE:
            on_mouse(&ev, cursor);
            break;
        case EVENT_TIMER:
            on_timer(&ev);
            break;
        }
    }
}
