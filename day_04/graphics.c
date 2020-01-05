#include <stdint.h>

#include "hello.h"
#include "graphics.h"


char *get_vram() {
    return *(char**)(0x0ff8);
}

int get_screen_x() {
    return *(int16_t*)(0x0ff4);
}

int get_screen_y() {
    return *(int16_t*)(0x0ff6);
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

void draw_pixel(int x, int y, int color) {
    get_vram()[y * get_screen_x() + x] = color;
}

void draw_rect(int x, int y, int width, int height, int color) {
    for (int y_ = y; y_ <= y+height; y_++) {
        for (int x_ = x; x_ <= x+width; x_++) {
            draw_pixel(x_, y_, color);
        }
    }
}
