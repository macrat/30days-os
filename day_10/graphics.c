#include <stdint.h>

#include "graphics.h"
#include "hello.h"
#include "math.h"
#include "memory.h"
#include "strings.h"


#define SCREEN_SPRITE_MAX  256
#define REDRAW_MARK_MAX    256

typedef struct Rectangle {
    uint32_t x, y, width, height;
} rectangle_t;

typedef struct RedrawMark {
    rectangle_t window;
    sprite_t*   source;
} redraw_mark_t;

typedef struct Screen {
    sprite_t      *first, *end;
    sprite_t      pool[SCREEN_SPRITE_MAX];
    uint32_t      need_redraw_count;
    redraw_mark_t need_redraw[REDRAW_MARK_MAX];
} screen_t;

static screen_t* _screen;


void init_screen() {
    _screen = malloc(sizeof(screen_t));
    memset(_screen, 0x00, sizeof(screen_t));
}

const color_t SIMPLE_COLORS[] = {
    {0x00, 0x00, 0x00},
    {0xff, 0x00, 0x00},
    {0x00, 0xff, 0x00},
    {0xff, 0xff, 0x00},
    {0x00, 0x00, 0xff},
    {0xff, 0x00, 0xff},
    {0x00, 0xff, 0xff},
    {0xff, 0xff, 0xff},
    {0xc6, 0xc6, 0xc6},
    {0x64, 0x00, 0x00},
    {0x00, 0x64, 0x00},
    {0x64, 0x64, 0x00},
    {0x00, 0x00, 0x64},
    {0x64, 0x00, 0x64},
    {0x00, 0x64, 0x64},
    {0x64, 0x64, 0x64},
};

void set_color(uint8_t id, const color_t color) {
    io_out8(0x03c8, id);
    io_out8(0x03c9, color.R / 4);
    io_out8(0x03c9, color.G / 4);
    io_out8(0x03c9, color.B / 4);
}

void set_palette(const color_t colors[]) {
    const int eflags = io_load_eflags();
    asm("CLI");
    for (uint_fast8_t i = 0; i < 16; i++) {
        set_color(i, colors[i]);
    }
    io_store_eflags(eflags);
}

static void reorder_sprite(sprite_t* sprite) {
    if (_screen->first == 0 || _screen->end == 0) {
        sprite->next = 0;
        _screen->first = _screen->end = sprite;
        return;
    }

    if (_screen->first == sprite) {
        _screen->first = sprite->next;
        if (_screen->first == 0) _screen->end = 0;
    }
    if (_screen->end == sprite) {
        sprite_t* itr = _screen->first;
        while (itr->next != sprite) itr = itr->next;
        _screen->end = itr;
    }

    bool drop = false, insert = false;

    if (_screen->first->order > sprite->order) {
        sprite->next = _screen->first;
        _screen->first = sprite;
        insert = true;
    }

    for (sprite_t* itr = _screen->first; itr->next != 0; itr = itr->next) {
        if (itr->next == sprite) {
            itr->next = sprite->next;
            drop = true;
        }
        if (!insert && itr->next->order > sprite->order) {
            sprite->next = itr->next;
            itr->next = sprite;
            insert = true;
        }
        if (drop && insert) {
            return;
        }
    }

    if (!insert) {
        _screen->end->next = sprite;
        _screen->end = sprite;
        sprite->next = 0;
    }
}

static void draw_sprite(sprite_t* sprite, rectangle_t window) {
    uint8_t* const vram = get_bootinfo()->vram;

    const uint32_t top    = max(sprite->y, window.y);
    const uint32_t bottom = min(get_screen_height(), min(sprite->y + sprite->height, window.y + window.height));
    const uint32_t left   = max(sprite->x, window.x);
    const uint32_t right  = min(get_screen_width(), min(sprite->x + sprite->width, window.x + window.width));

    for (uint32_t global_y = top; global_y < bottom; global_y++) {
        for (uint32_t global_x = left; global_x < right; global_x++) {
            const uint32_t local_x = global_x - sprite->x;
            const uint32_t local_y = global_y - sprite->y;

            const uint8_t color = sprite->image[local_y * sprite->width + local_x];
            if (color > 16) continue;

            vram[global_y * get_screen_width() + global_x] = color;
        }
    }
}

void refresh_screen() {
    for (uint32_t i = 0; i < _screen->need_redraw_count; i++) {
        for (sprite_t* itr = _screen->need_redraw[i].source; itr != 0; itr = itr->next) {
            draw_sprite(itr, _screen->need_redraw[i].window);
        }
    }
    _screen->need_redraw_count = 0;
}

#define equals_rect(a, b)  ((a).x == (b).x && (a).y == (b).y && (a).width == (b).width && (a).height == (b).height)

static void mark_as_need_redraw(sprite_t* sprite, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    const rectangle_t window = {sprite->x + x, sprite->y + y, width, height};

    if (_screen->need_redraw_count > 0 && equals_rect(_screen->need_redraw[_screen->need_redraw_count - 1].window, window)) {
        return;
    }

    if (_screen->need_redraw_count >= REDRAW_MARK_MAX) {
        refresh_screen();
    }

    _screen->need_redraw[_screen->need_redraw_count].window = window;
    _screen->need_redraw[_screen->need_redraw_count].source = sprite;
    _screen->need_redraw_count++;
}

void move_sprite(sprite_t* sprite, uint32_t x, uint32_t y, uint8_t order) {
    const bool need_reorder = sprite->order != order;

    mark_as_need_redraw(_screen->first, sprite->x, sprite->y, sprite->width, sprite->height);

    sprite->x = x;
    sprite->y = y;
    sprite->order = order;

    if (need_reorder) reorder_sprite(sprite);

    mark_as_need_redraw(_screen->first, sprite->x, sprite->y, sprite->width, sprite->height);
}

sprite_t* create_sprite(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t order) {
    sprite_t* allocated = 0;
    for (uint_fast16_t i = 0; i < SCREEN_SPRITE_MAX; i++) {
        if (!_screen->pool[i].active) {
            allocated = &_screen->pool[i];
            break;
        }
    }
    if (allocated == 0) return 0;

    allocated->active = true;
    allocated->x = x;
    allocated->y = y;
    allocated->width = width;
    allocated->height = height;
    allocated->order = order;
    allocated->image = malloc(width * height);
    allocated->next = 0;

    memset(allocated->image, COLOR_TRANSPARENT, width * height);

    reorder_sprite(allocated);

    return allocated;
}

void destroy_sprite(sprite_t* sprite) {
    sprite->active = false;

    if (_screen->first == 0) return;

    if (_screen->first == sprite) {
        if (_screen->end == sprite) {
            _screen->first = _screen->end = 0;
            return;
        }

        _screen->first = sprite->next;
        return;
    }

    if (_screen->end == sprite) {
        sprite_t* itr = _screen->first;
        for (; itr->next != sprite; itr = itr->next);
        itr->next = 0;
        _screen->end = itr;
        return;
    }

    for (sprite_t* itr = _screen->first; itr != 0; itr = itr->next) {
        if (itr->next == sprite) {
            itr->next = itr->next->next;
            break;
        }
    }
}

void fill_sprite(sprite_t* sprite, uint8_t color) {
    mark_as_need_redraw(sprite, 0, 0, sprite->width, sprite->height);
    memset(sprite->image, color, sprite->width * sprite->height);
}

static void __draw_pixel(sprite_t* sprite, uint32_t x, uint32_t y, uint8_t color) {
    if (sprite->width <= x || sprite->height <= y) {
        return;
    }

    sprite->image[y * sprite->width + x] = color;
}

void draw_pixel(sprite_t* sprite, uint32_t x, uint32_t y, uint8_t color) {
    mark_as_need_redraw(sprite, x, y, 1, 1);
    __draw_pixel(sprite, x, y, color);
}

void draw_rect(sprite_t* sprite, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t color) {
    mark_as_need_redraw(sprite, x, y, width, height);

    for (uint32_t y_ = y; y_ <= y+height; y_++) {
        for (uint32_t x_ = x; x_ <= x+width; x_++) {
            __draw_pixel(sprite, x_, y_, color);
        }
    }
}

void draw_char(sprite_t* sprite, uint32_t x, uint32_t y, char character, const uint8_t* font, uint8_t color) {
    mark_as_need_redraw(sprite, x, y, 8, 16);

    for (uint_fast8_t i = 0; i < 8*16; i++) {
        if (font[character*16 + i/8] & (1<<(8-i%8))) {
            __draw_pixel(sprite, x + i%8, y + i/8, color);
        }
    }
}

void draw_text(sprite_t* sprite, uint32_t x, uint32_t y, char* text, const uint8_t* font, uint8_t color) {
    for (uint32_t xshift = 0; *text != 0; text++, xshift++) {
        switch (*text) {
        case '\t':
            xshift += 3 - xshift%4;
            continue;
        case '\n':
            y += 16;
            xshift = -1;
            continue;
        }
        draw_char(sprite, x + xshift*8, y, *text, font, color);
    }
}

void draw_int(sprite_t* sprite, uint32_t x, uint32_t y, int value, uint8_t radix, const uint8_t* font, uint8_t color) {
    char buf[1024];
    draw_text(sprite, x, y, itoa_s(value, buf, sizeof(buf), radix), font, color);
}

void draw_image(sprite_t* sprite, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const uint8_t* image) {
    mark_as_need_redraw(sprite, x, y, width, height);

    for (uint_fast32_t i = 0; i < width*height; i++, image++) {
        __draw_pixel(sprite, x + i%width, y + i/width, *image);
    }
}

sprite_t* create_cursor_sprite() {
#define x COLOR_WHITE,
#define B COLOR_BLACK,
#define _ COLOR_TRANSPARENT,

    static const uint8_t cursor[16 * 16] = {
        B B B B B B B B B B B B B B B B
        B x x x x x x x x x x x x x B _
        B x x x x x x x x x x x x B _ _
        B x x x x x x x x x x x B _ _ _
        B x x x x x x x x x x B _ _ _ _
        B x x x x x x x x x B _ _ _ _ _
        B x x x x x x x x x B _ _ _ _ _
        B x x x x x x x x x x B _ _ _ _
        B x x x x x x x x x x x B _ _ _
        B x x x x x x x x x x x x B _ _
        B x x x x B B x x x x x x x B _
        B x x x B _ _ B x x x x x x x B
        B x x B _ _ _ _ B x x x x x B _
        B x B _ _ _ _ _ _ B x x x B _ _
        B B _ _ _ _ _ _ _ _ B x B _ _ _
        B _ _ _ _ _ _ _ _ _ _ B _ _ _ _
    };

#undef x
#undef B
#undef _

    sprite_t* sprite = create_sprite(get_screen_width()/2, get_screen_height()/2, 16, 16, 255);
    draw_image(sprite, 0, 0, 16, 16, cursor);
    return sprite;
}

void move_cursor_sprite(sprite_t* sprite, const mouse_state_t* state) {
    move_sprite(sprite, state->x, state->y, 255);
}
