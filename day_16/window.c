#include <stdlib.h>
#include <string.h>

#include "math.h"
#include "window.h"

#include "hankaku.h" // debug


#define WINDOW_TOP_ORDER 200
#define WINDOW_MAX_NUM   128

typedef struct WindowManager {
    window_t *focused;
    window_t pool[WINDOW_MAX_NUM];
} window_manager_t;

static sprite_t* _cursor;
static window_manager_t* _window_manager;


void init_window_manager() {
    _cursor = create_cursor_sprite();

    _window_manager = malloc(sizeof(window_manager_t));
    memset(_window_manager, 0x00, sizeof(window_manager_t));
}

static window_t* allocate_window() {
    for (int i = 0; i < WINDOW_MAX_NUM; i++) {
        if (!_window_manager->pool[i].active) {
            _window_manager->pool[i].active = true;
            return &_window_manager->pool[i];
        }
    }
    return 0;
}

window_t* create_window(uint32_t width, uint32_t height) {
    window_t* window = allocate_window();
    if (window == 0) return 0;

    sprite_t* fg = create_sprite(
        get_screen_width()/2 - width/2,
        get_screen_height()/2 - height/2,
        width,
        height,
        0
    );
    if (fg == 0) {
        window->active = false;
        return 0;
    }
    sprite_t* bg = create_sprite(
        get_screen_width()/2 - width/2 - WINROD_BORDER_TICKNESS,
        get_screen_height()/2 - height/2 - WINROD_BORDER_TICKNESS,
        width + WINROD_BORDER_TICKNESS * 2,
        height + WINROD_BORDER_TICKNESS * 2,
        0
    );
    if (bg == 0) {
        destroy_sprite(fg);
        window->active = false;
        return 0;
    }

    window->fg = fg;
    window->bg = bg;
    window->on_keyboard = 0;

    clear_window(window);
    focus_window(window);

    return window;
}

static void draw_window_background(window_t* window) {
    const uint8_t color = _window_manager->focused == window ? COLOR_RED : COLOR_DARK_GRAY;

    fill_sprite(window->bg, color);
    draw_rect(
        window->bg,
        WINROD_BORDER_TICKNESS,
        WINROD_BORDER_TICKNESS,
        window->bg->width - WINROD_BORDER_TICKNESS*2,
        window->bg->height - WINROD_BORDER_TICKNESS*2,
        COLOR_WHITE
    );
}

static inline uint_fast32_t get_window_order(window_t* window) {
    return window->fg->olist_item->order;
}

static void reorder_window_to(window_t* window, int8_t order) {
    move_sprite(window->fg, window->fg->x, window->fg->y, order);
    move_sprite(window->bg, window->bg->x, window->bg->y, order - 1);
}

static void reorder_window_by(window_t* window, int8_t offset) {
    reorder_window_to(window, get_window_order(window) + offset * 2);
}

void destroy_window(window_t* window) {
    for (int i = 0; i < WINDOW_MAX_NUM; i++) {
        window_t* target = &_window_manager->pool[i];

        if (target->active && get_window_order(target) < get_window_order(window)) {
            reorder_window_by(target, 1);
            draw_window_background(target);
        }
    }

    destroy_sprite(window->fg);
    destroy_sprite(window->bg);
    window->active = false;
}

void focus_window(window_t* window) {
    if (get_window_order(window) == WINDOW_TOP_ORDER && _window_manager->focused == window) return;

    const window_t* const old_focused = _window_manager->focused;
    _window_manager->focused = window;

    for (int i = 0; i < WINDOW_MAX_NUM; i++) {
        window_t* target = &_window_manager->pool[i];

        if (target->active && target != window && get_window_order(target) >= get_window_order(window)) {
            reorder_window_by(target, -1);

            if (target == old_focused) {
                draw_window_background(target);
            }
        }
    }

    reorder_window_to(window, WINDOW_TOP_ORDER);
    draw_window_background(window);
}

void move_window(window_t* window, int32_t x, int32_t y) {
    x = max(WINROD_BORDER_TICKNESS - (int32_t)window->bg->width, min(get_screen_width() - WINROD_BORDER_TICKNESS, x));
    y = max(WINROD_BORDER_TICKNESS - (int32_t)window->bg->height, min(get_screen_height() - WINROD_BORDER_TICKNESS, y));

    move_sprite(window->fg, x + WINROD_BORDER_TICKNESS, y + WINROD_BORDER_TICKNESS, get_window_order(window));
    move_sprite(window->bg, x, y, get_window_order(window) - 1);
}

void clear_window(window_t* window) {
    fill_sprite(window->fg, COLOR_TRANSPARENT);
    draw_window_background(window);
}

void on_keyboard(const event_t* ev) {
    window_t* const focused = _window_manager->focused;
    if (focused->on_keyboard) {
        focused->on_keyboard((keyboard_event_t*)&ev->data, focused);
    }
}

static window_t* find_window_by_position(int32_t x, int32_t y) {
    window_t* window = 0;
    uint8_t order = 0;

    for (int i = 0; i < WINDOW_MAX_NUM; i++) {
        window_t* target = &_window_manager->pool[i];
        if (
            target->active
            && order <= get_window_order(target)
            && target->bg->x <= x && x <= target->bg->x + (int32_t)target->bg->width
            && target->bg->y <= y && y <= target->bg->y + (int32_t)target->bg->height
        ) {
            window = target;
            order = get_window_order(target);
        }
    }

    return window;
}

void on_mouse(const event_t* ev) {
    static bool pressed = false;
    static window_t* grabbed = 0;
    static int32_t offset_x, offset_y;

    const mouse_state_t* const me = (mouse_state_t*)&ev->data;

    move_cursor_sprite(_cursor, me);

    if (grabbed) {
        move_window(grabbed, me->x + offset_x, me->y + offset_y);
    }

    if ((me->buttons & MOUSE_LEFT) == 0) {
        grabbed = 0;
        pressed = false;
    } else if (!pressed) {
        pressed = true;

        window_t* win = find_window_by_position(me->x, me->y);
        if (win) {
            focus_window(win);
            grabbed = win;
            offset_x = win->bg->x - me->x;
            offset_y = win->bg->y - me->y;
        }
    }
}
