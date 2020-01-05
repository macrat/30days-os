#pragma once

#include <stdint.h>

#define COLOR_BLACK        (0)
#define COLOR_RED          (1)
#define COLOR_GREEN        (2)
#define COLOR_YELLOW       (3)
#define COLOR_BLUE         (4)
#define COLOR_PURPLE       (5)
#define COLOR_CYAN         (6)
#define COLOR_WHITE        (7)
#define COLOR_LIGHT_GRAY   (8)
#define COLOR_DARK_RED     (8+1)
#define COLOR_DARK_GREEN   (8+2)
#define COLOR_DARK_BLUE    (8+4)
#define COLOR_DARK_YELLOW  (8+5)
#define COLOR_DARK_PURPLE  (8+6)
#define COLOR_DARK_CYAN    (8+7)
#define COLOR_DARK_GRAY    (8+8)

struct Color {
    uint8_t R, G, B;
} typedef color_t;

extern const color_t SIMPLE_COLORS[];

extern int get_screen_x();
extern int get_screen_y();

extern void set_color(int id, const color_t color);
extern void set_palette(const color_t colors[]);

extern void draw_pixel(int x, int y, int color);
extern void draw_rect(int x, int y, int width, int height, int color);
