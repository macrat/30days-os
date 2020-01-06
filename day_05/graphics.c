#include <stdint.h>

#include "hello.h"
#include "graphics.h"


uint8_t *get_vram() {
    return get_bootinfo()->vram;
}

int get_screen_x() {
    return get_bootinfo()->screen_x;
}

int get_screen_y() {
    return get_bootinfo()->screen_y;
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

void set_color(int id, const color_t color) {
    io_out8(0x03c8, id);
    io_out8(0x03c9, color.R / 4);
    io_out8(0x03c9, color.G / 4);
    io_out8(0x03c9, color.B / 4);
}

void set_palette(const color_t colors[]) {
    const int eflags = io_load_eflags();
    io_cli();
    for (int i = 0; i < 16; i++) {
        set_color(i, colors[i]);
    }
    io_store_eflags(eflags);
}

void draw_pixel(int x, int y, uint8_t color) {
    if (color <= 16) {
        get_vram()[y * get_screen_x() + x] = color;
    }
}

void draw_rect(int x, int y, int width, int height, uint8_t color) {
    for (int y_ = y; y_ <= y+height; y_++) {
        for (int x_ = x; x_ <= x+width; x_++) {
            draw_pixel(x_, y_, color);
        }
    }
}

void draw_char(int x, int y, char character, const uint8_t *font, uint8_t color) {
    for (int i = 0; i < 8*16; i++) {
        if (font[character*16 + i/8] & (1<<(8-i%8))) {
            draw_pixel(x + i%8, y + i/8, color);
        }
    }
}

void draw_text(int x, int y, char *text, const uint8_t *font, uint8_t color) {
    for (int xshift = 0; *text != 0; text++, xshift++) {
        switch (*text) {
        case '\t':
            xshift += 3 - xshift%4;
            continue;
        case '\n':
            y += 16;
            xshift = -1;
            continue;
        }
        draw_char(x + xshift*8, y, *text, font, color);
    }
}

void draw_image(int x, int y, int width, int height, uint8_t *image) {
    for (int i = 0; i < width*height; i++, image++) {
        draw_pixel(x + i%width, y + i/width, *image);
    }
}

void draw_mouse_cursor(int x, int y) {
#define x COLOR_WHITE,
#define B COLOR_BLACK,
#define _ COLOR_TRANSPARENT,

    static uint8_t cursor[16 * 16] = {
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

    draw_image(x, y, 16, 16, cursor);
}
