#include <stdint.h>

#include "graphics.h"
#include "hello.h"
#include "math.h"
#include "memory.h"
#include "strings.h"


#define SCREEN_SPRITE_MAX  256
#define REDRAW_MARK_MAX    256

typedef struct Rectangle {
    int32_t  x, y;
    uint32_t width, height;
} rectangle_t;

typedef struct RedrawMark {
    rectangle_t window;
    sprite_t*   source;
} redraw_mark_t;

typedef struct Screen {
    uint8_t*      buffer;
    sprite_t*     first;
    sprite_t      pool[SCREEN_SPRITE_MAX];
    uint32_t      need_redraw_count;
    redraw_mark_t need_redraw[REDRAW_MARK_MAX];
} screen_t;

static screen_t* _screen;


void init_screen() {
    _screen = malloc(sizeof(screen_t));
    memset(_screen, 0x00, sizeof(screen_t));

    _screen->buffer = malloc(get_screen_width() * get_screen_height());
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

static void drop_sprite(sprite_t* sprite) {
    if (_screen->first == 0) return;

    if (_screen->first == sprite) {
        _screen->first = sprite->next;
        return;
    }

    for (sprite_t* itr = _screen->first; itr->next != 0; itr = itr->next) {
        if (itr->next == sprite) {
            itr->next = sprite->next;
            return;
        }
    }
}

static void reorder_sprite(sprite_t* sprite) {
    drop_sprite(sprite);

    sprite_t** itr = &_screen->first;
    while (*itr && (*itr)->order < sprite->order) {
        if ((*itr)->next == sprite) {
            (*itr)->next = sprite->next;
            return;
        }

        itr = &(*itr)->next;
    }
    sprite->next = *itr;
    *itr = sprite;
}

static void draw_sprite(sprite_t* sprite, rectangle_t window) {
    const int32_t top    = max(0, max(sprite->y, window.y));
    const int32_t bottom = min(get_screen_height(), min(sprite->y + sprite->height, window.y + window.height));
    const int32_t left   = max(0, max(sprite->x, window.x));
    const int32_t right  = min(get_screen_width(), min(sprite->x + sprite->width, window.x + window.width));

    for (int32_t global_y = top; global_y < bottom; global_y++) {
        for (int32_t global_x = left; global_x < right; global_x++) {
            const int32_t local_x = global_x - sprite->x;
            const int32_t local_y = global_y - sprite->y;

            const uint8_t color = sprite->image[local_y * sprite->width + local_x];
            if (color > 16) continue;

            _screen->buffer[global_y * get_screen_width() + global_x] = color;
        }
    }
}

static void copy_image(uint8_t* dest, uint8_t* src, rectangle_t window) {
    for (uint32_t y = max(0, window.y); y < max(0, window.y + (int32_t)window.height); y++) {
        for (uint32_t x = max(0, window.x); x < max(0, window.x + (int32_t)window.width); x++) {
            dest[y * get_screen_width() + x] = src[y * get_screen_width() + x];
        }
    }
}

void refresh_screen() {
    for (uint32_t i = 0; i < _screen->need_redraw_count; i++) {
        for (sprite_t* itr = _screen->need_redraw[i].source; itr != 0; itr = itr->next) {
            draw_sprite(itr, _screen->need_redraw[i].window);
        }
    }
    for (uint32_t i = 0; i < _screen->need_redraw_count; i++) {
        copy_image(get_bootinfo()->vram, _screen->buffer, _screen->need_redraw[i].window);
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

void move_sprite(sprite_t* sprite, int32_t x, int32_t y, uint8_t order) {
    const bool need_reorder = sprite->order != order;

    mark_as_need_redraw(_screen->first, sprite->x, sprite->y, sprite->width, sprite->height);

    sprite->x = x;
    sprite->y = y;
    sprite->order = order;

    if (need_reorder) reorder_sprite(sprite);

    mark_as_need_redraw(_screen->first, sprite->x, sprite->y, sprite->width, sprite->height);
}

sprite_t* create_sprite(int32_t x, int32_t y, uint32_t width, uint32_t height, uint8_t order) {
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

    if (allocated->image == 0) {
        drop_sprite(allocated);
        allocated->active = false;
        return 0;
    }

    memset(allocated->image, COLOR_TRANSPARENT, width * height);

    reorder_sprite(allocated);

    return allocated;
}

void destroy_sprite(sprite_t* sprite) {
    free(sprite->image);
    drop_sprite(sprite);
    sprite->active = false;
}

int count_sprites() {
    int count = 0;
    for (sprite_t* itr = _screen->first; itr; itr = itr->next) {
        count++;
    }
    return count;
}

void fill_sprite(sprite_t* sprite, uint8_t color) {
    mark_as_need_redraw(sprite, 0, 0, sprite->width, sprite->height);
    memset(sprite->image, color, sprite->width * sprite->height);
}

static void __draw_pixel(sprite_t* sprite, int32_t x, int32_t y, uint8_t color) {
    if (x < 0 || (int32_t)sprite->width <= x || y < 0 || (int32_t)sprite->height <= y) {
        return;
    }

    sprite->image[y * sprite->width + x] = color;
}

void draw_pixel(sprite_t* sprite, int32_t x, int32_t y, uint8_t color) {
    mark_as_need_redraw(sprite, x, y, 1, 1);
    __draw_pixel(sprite, x, y, color);
}

void draw_rect(sprite_t* sprite, int32_t x, int32_t y, uint32_t width, uint32_t height, uint8_t color) {
    mark_as_need_redraw(sprite, x, y, width, height);

    for (int32_t y_ = y; y_ < (int32_t)(y+height); y_++) {
        for (int32_t x_ = x; x_ < (int32_t)(x+width); x_++) {
            __draw_pixel(sprite, x_, y_, color);
        }
    }
}

void draw_char(sprite_t* sprite, int32_t x, int32_t y, char character, const uint8_t* font, uint8_t fg, uint8_t bg) {
    mark_as_need_redraw(sprite, x, y, 8, 16);

    for (uint_fast8_t i = 0; i < 8*16; i++) {
        __draw_pixel(sprite, x + i%8, y + i/8, (font[character*16 + i/8] & (1<<(8-i%8))) ? fg : bg);
    }
}

void draw_text(sprite_t* sprite, int32_t x, int32_t y, char* text, const uint8_t* font, uint8_t fg, uint8_t bg) {
    for (int32_t xshift = 0; *text != 0; text++, xshift++) {
        switch (*text) {
        case '\t':
            xshift += 3 - xshift%4;
            continue;
        case '\n':
            y += 16;
            xshift = -1;
            continue;
        }
        draw_char(sprite, x + xshift*8, y, *text, font, fg, bg);
    }
}

void draw_int(sprite_t* sprite, int32_t x, int32_t y, int value, uint8_t radix, const uint8_t* font, uint8_t fg, uint8_t bg) {
    char buf[1024];
    draw_text(sprite, x, y, itoa_s(value, buf, sizeof(buf), radix), font, fg, bg);
}

void draw_image(sprite_t* sprite, int32_t x, int32_t y, uint32_t width, uint32_t height, const uint8_t* image) {
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
