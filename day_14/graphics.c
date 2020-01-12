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

static uint8_t* buffer = 0;
static olist_t* sprites = 0;
static olist_t* redraw_marks = 0;


int init_screen() {
    buffer = malloc(get_screen_width() * get_screen_height());
    sprites = create_olist(SCREEN_SPRITE_MAX, sizeof(sprite_t));
    redraw_marks = create_olist(REDRAW_MARK_MAX, sizeof(rectangle_t));

    if (buffer == 0 || sprites == 0 || redraw_marks == 0) {
        free(buffer);
        free(sprites);
        free(redraw_marks);
        return -1;
    }

    return 0;
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
    __asm__("CLI");
    for (uint_fast8_t i = 0; i < 16; i++) {
        set_color(i, colors[i]);
    }
    io_store_eflags(eflags);
}

static void draw_sprite(const sprite_t* sprite, rectangle_t window) {
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

            buffer[global_y * get_screen_width() + global_x] = color;
        }
    }
}

static void copy_image(uint8_t* dest, const uint8_t* src, rectangle_t window) {
    for (uint32_t y = max(0, window.y); y < max(0, window.y + (int32_t)window.height); y++) {
        for (uint32_t x = max(0, window.x); x < max(0, window.x + (int32_t)window.width); x++) {
            dest[y * get_screen_width() + x] = src[y * get_screen_width() + x];
        }
    }
}

void refresh_screen() {
    if (!redraw_marks->first) return;

    for (olist_item_t* mitr = redraw_marks->first; mitr; mitr = mitr->next) {
        const rectangle_t* const mark = (rectangle_t*)mitr->payload;

        for (olist_item_t* sitr = sprites->first; sitr; sitr = sitr->next) {
            draw_sprite((sprite_t*)sitr->payload, *mark);
        }
    }

    rectangle_t buf;
    while (olist_pop(redraw_marks, &buf, 0)) {
        copy_image(get_bootinfo()->vram, buffer, buf);
    }
}

static inline bool rect_is_overwrap(rectangle_t a, rectangle_t b) {
    return (
        a.x <= b.x + (int32_t)b.width && b.x + (int32_t)b.width <= a.x + (int32_t)a.width + (int32_t)b.width
        && a.y <= b.y + (int32_t)b.height && b.y + (int32_t)b.height <= a.y + (int32_t)a.height + (int32_t)b.height
    );
}

static void push_redraw_mark(rectangle_t window) {
    for (olist_item_t* itr = redraw_marks->first; itr; itr = itr->next) {
        const rectangle_t* const mark = (rectangle_t*)itr->payload;

        if (rect_is_overwrap(*mark, window)) {
            if (mark->x < window.x) {
                window.width += window.x - mark->x;
                window.x = mark->x;
            }
            if (mark->y < window.y) {
                window.height += window.y - mark->y;
                window.y = mark->y;
            }
            if (mark->x + mark->width > window.x + window.width) {
                window.width = mark->x + mark->width - window.x;
            }
            if (mark->y + mark->height > window.y + window.height) {
                window.height = mark->y + mark->height - window.y;
            }

            olist_drop(redraw_marks, itr);
            push_redraw_mark(window);
            return;
        }
    }

    const uint_fast32_t order = get_screen_width() * get_screen_height() - window.width * window.height;

    if (olist_push(redraw_marks, order, &window) == 0) {
        refresh_screen();
        olist_push(redraw_marks, order, &window);
    }
}

static void mark_as_need_redraw(sprite_t* sprite, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    push_redraw_mark((rectangle_t){sprite->x + x, sprite->y + y, width, height});
}

void move_sprite(sprite_t* sprite, int32_t x, int32_t y, uint8_t order) {
    mark_as_need_redraw((sprite_t*)sprites->first->payload, sprite->x, sprite->y, sprite->width, sprite->height);

    sprite->x = x;
    sprite->y = y;

    olist_reorder(sprites, sprite->olist_item, order);

    mark_as_need_redraw((sprite_t*)sprites->first->payload, sprite->x, sprite->y, sprite->width, sprite->height);
}

sprite_t* create_sprite(int32_t x, int32_t y, uint32_t width, uint32_t height, uint8_t order) {
    uint8_t* buf = malloc(width * height);
    if (!buf) return 0;

    memset(buf, COLOR_TRANSPARENT, width * height);

    olist_item_t* allocated = olist_push(sprites, order, &(sprite_t){
        .x      = x,
        .y      = y,
        .width  = width,
        .height = height,
        .image  = buf,
    });
    if (allocated == 0) {
        free(buf);
        return 0;
    }

    ((sprite_t*)allocated->payload)->olist_item = allocated;

    return (sprite_t*)allocated->payload;
}

void destroy_sprite(sprite_t* sprite) {
    olist_drop(sprites, sprite->olist_item);
    free(sprite->image);
}

int count_sprites() {
    return olist_item_count(sprites);
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
