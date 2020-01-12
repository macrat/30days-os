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
#include "task.h"
#include "timer.h"
#include "window.h"


uint32_t io_load_eflags() {
    uint32_t eax = 0;
    __asm__(
        "PUSHFL;"
        "POP  %%EAX;"
        :
        : "a"(eax)
    );
    return eax;
}

void io_store_eflags(uint32_t eflags) {
    __asm__(
        "PUSH  %%EAX;"
        "POPFL;"
        : "=a"(eflags)
    );
}

sprite_t* create_desktop() {
    sprite_t* desktop = create_sprite(0, 0, get_screen_width(), get_screen_height(), 0);

    draw_rect(desktop, 0, 0, get_screen_width(), get_screen_height(), COLOR_WHITE);
    draw_rect(desktop, 20, 20, 30, 30, COLOR_RED);

    draw_text(desktop, 55, 20, "012345678901234567890\nhello\tworld!\nthis\tis\ta\ttest", get_hankaku_font(), COLOR_RED, COLOR_WHITE);

    return desktop;
}

void draw_keyboard_input(const keyboard_event_t* ev, window_t* window) {
    static int receive_count = 0;
    receive_count++;

    const uint32_t padding = 2;

    draw_char(window->fg, padding, padding, ev->character, get_hankaku_font(), COLOR_DARK_GRAY, COLOR_WHITE);
    draw_int(window->fg, padding+8*3, padding, ev->keycode, 16, get_hankaku_font(), COLOR_DARK_GRAY, COLOR_WHITE);
    draw_int(window->fg, padding+8*6, padding, ev->modifier, 16, get_hankaku_font(), COLOR_DARK_GRAY, COLOR_WHITE);
    draw_int(window->fg, padding, padding+16, receive_count, 10, get_hankaku_font(), COLOR_DARK_GRAY, COLOR_WHITE);
}

typedef struct TextEditor {
    char buf[1024];
    int  cursor;
    bool show_cursor;
} text_editor_t;

void draw_text_editor(window_t* window) {
    fill_sprite(window->fg, COLOR_TRANSPARENT);

    text_editor_t* editor = (text_editor_t*)window->extra;
    if (editor->show_cursor) {
        editor->buf[editor->cursor] = '|';
        editor->buf[editor->cursor+1] = 0x00;
    } else {
        editor->buf[editor->cursor] = 0x00;
    }

    draw_text(window->fg, 3, 3, editor->buf, get_hankaku_font(), COLOR_BLACK, COLOR_TRANSPARENT);
}

void text_editor_on_input(const keyboard_event_t* ev, window_t* window) {
    if (ev->type != KEY_PRESS) return;

    text_editor_t* editor = (text_editor_t*)window->extra;

    if (ev->character == '\b') {
        if (editor->cursor > 0) editor->buf[--editor->cursor] = '\0';
    } else if (ev->character != 0) {
        editor->buf[editor->cursor++] = ev->character;
    }

    draw_text_editor(window);
}

void text_editor_on_tick(void* data) {
    window_t* window = (window_t*)data;
    text_editor_t* editor = (text_editor_t*)window->extra;
    editor->show_cursor = !editor->show_cursor;

    draw_text_editor(window);
    set_timeout(text_editor_on_tick, data, 50);
}

window_t* create_text_editor() {
    text_editor_t* const editor = malloc(sizeof(text_editor_t));
    memset(editor, 0x00, sizeof(text_editor_t));

    window_t* const win = create_window(200, 100);
    win->extra = editor;
    win->on_keyboard = text_editor_on_input;

    set_timeout(text_editor_on_tick, (void*)win, 50);

    return win;
}

#define BENCHMARK_NUM      6
#define BENCHMARK_DURATION 10

typedef struct Benchmark {
    uint32_t  counter;
    uint32_t  history[BENCHMARK_NUM];
    int8_t    index;
} benchmark_t;

void benchmark_on_timeout(void* ptr) {
    window_t* window = (window_t*)ptr;
    benchmark_t* bm = (benchmark_t*)window->extra;

    if (bm->index >= 0) {
        bm->history[bm->index] = bm->counter;

        for (int line = 0; line <= bm->index; line++) {
            char buf[20];
            size_t len = strlen(itoa_s(bm->history[line] / BENCHMARK_DURATION, buf, sizeof(buf), 10));
            strcpy_s(buf + len, sizeof(buf) - len, "ops/sec");
            pad_right(buf, ' ', 16);

            draw_text(window->fg, 3, 3 + 16*line, buf, get_hankaku_font(), COLOR_BLACK, COLOR_WHITE);
        }
    }

    bm->index++;

    bm->counter = 0;
    if (bm->index < BENCHMARK_NUM) {
        set_timeout(benchmark_on_timeout, ptr, BENCHMARK_DURATION*SECOND);
    }
}

void benchmark_on_keyboard(const keyboard_event_t* ev, window_t* window) {
    if (ev->type == KEY_PRESS && ev->character == ' ') {
        benchmark_t* bm = (benchmark_t*)window->extra;

        if (bm->index >= BENCHMARK_NUM) {
            set_timeout(benchmark_on_timeout, window, 3*SECOND);
        }
        bm->index = -1;

        fill_sprite(window->fg, COLOR_TRANSPARENT);
        draw_text(window->fg, 3, 3, "restarting...", get_hankaku_font(), COLOR_DARK_GRAY, COLOR_TRANSPARENT);
    }
}

void benchmark_task(void* ptr) {
    benchmark_t* bm = (benchmark_t*)ptr;

    while (1) {
        bm->counter++;
        //__asm__("HLT");
    }
}

task_t* create_benchmark(uint32_t x, uint32_t y) {
    benchmark_t* bm = malloc(sizeof(benchmark_t));
    memset(bm, 0x00, sizeof(benchmark_t));
    bm->index = -1;  // drop first

    window_t* win = create_window(8 * 16 + 3*2, 16 * BENCHMARK_NUM + 3*2);
    move_window(win, x, y);
    win->extra = (void*)bm;
    win->on_keyboard = benchmark_on_keyboard;
    set_timeout(benchmark_on_timeout, (void*)win, 3*SECOND);

    return task_start(benchmark_task, bm);
}

void counter_window_timeout(window_t* win) {
    static uint8_t counter;
    counter++;

    draw_rect(win->fg, 0, 0, 20, 20, counter%16);
    draw_int(win->fg, 20+3, 3, (int)win->extra, 10, get_hankaku_font(), COLOR_BLACK, COLOR_TRANSPARENT);

    set_timeout((timeout_handler_t)counter_window_timeout, (void*)win, SECOND/10);
}

int* create_counter_window(uint32_t x, uint32_t y) {
    window_t* win = create_window(100, 20);
    move_window(win, x, y);
    counter_window_timeout(win);

    return (int*)(&win->extra);
}

void task_count_window_timeout(window_t* win) {
    fill_sprite(win->fg, COLOR_TRANSPARENT);

    draw_int(win->fg, 3, 3, get_running_task_num(), 10, get_hankaku_font(), COLOR_BLACK, COLOR_TRANSPARENT);
    draw_char(win->fg, 50/2 - 16/2, 3, '/', get_hankaku_font(), COLOR_BLACK, COLOR_TRANSPARENT);
    draw_int(win->fg, 50/2 + 16/2, 3, get_active_task_num(), 10, get_hankaku_font(), COLOR_BLACK, COLOR_TRANSPARENT);
}

void create_task_count_window(uint32_t x, uint32_t y) {
    window_t* win = create_window(50, 20);
    move_window(win, x, y);
    task_count_window_timeout(win);
}

void main() {
    init_descriptor_table();
    init_pic();
    __asm__("STI");

    init_memory_manager();
    register_memory_area((void*)0x00001000, 0x0009e000);
    register_memory_area((void*)0x00400000, scan_memory_size((void*)0x00400000, (void*)0xbfffffff) - 0x00400000);

    init_event_queue();
    init_pit();
    enable_interrupt();
    init_input_devices();
    init_timeout_manager();
    init_task_manager();

    set_palette(SIMPLE_COLORS);
    init_screen();

    create_desktop();
    init_window_manager();

    window_t* keycard = create_window(8*10 + 3 * 2, 16*2 + 3 * 2);
    move_window(keycard, 10, get_screen_height() - keycard->bg->height - 10);
    keycard->on_keyboard = draw_keyboard_input;

    create_text_editor();

    create_benchmark(get_screen_width() - 8*16 - 30,  20);
    create_benchmark(get_screen_width() - 8*16 - 30, 150);

    for (int i=10; i<=100; i+=10) {
        move_window(
            create_window(i, i),
            get_screen_width() - 220 + i,
            get_screen_height() - 220 + i
        );
    }

    window_t* gradient_win = create_window(90, 90);
    for (int i=0; i<16; i++) {
        draw_rect(gradient_win->fg, 0, i*90/16, 90, (i+1)*90/16, i);
    }

    int* counter = create_counter_window(20, 80);
    create_task_count_window(20, 110);

    subscribe_event(0);

    while (1) {
        *counter = *counter + 1;

        event_t ev;
        __asm__("CLI");
        const event_t* const evp = pop_event();
        if (evp) ev = *evp;
        __asm__("STI");

        if (evp == 0) {
            task_sleep(0);
            //__asm__("HLT");
            continue;
        }

        switch (ev.type) {
        case EVENT_RAW_KEYBOARD:
            on_raw_keyboard(&ev);
            break;
        case EVENT_RAW_MOUSE:
            on_raw_mouse(&ev);
            break;
        case EVENT_KEYBOARD:
            on_keyboard(&ev);
            break;
        case EVENT_MOUSE:
            on_mouse(&ev);
            break;
        }
    }
}
