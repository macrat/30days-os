#pragma once

#include <stdint.h>

#include "input_devices.h"


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
#define COLOR_TRANSPARENT  (0xff)


struct Color {
    uint8_t R, G, B;
} typedef color_t;

extern const color_t SIMPLE_COLORS[];

extern uint16_t get_screen_x();
extern uint16_t get_screen_y();

extern void set_color(uint8_t id, const color_t color);
extern void set_palette(const color_t colors[]);

extern void draw_pixel(uint_fast16_t x, uint_fast16_t y, uint8_t color);
extern void draw_rect(uint_fast16_t x, uint_fast16_t y, uint_fast16_t width, uint_fast16_t height, uint8_t color);
extern void draw_char(uint_fast16_t x, uint_fast16_t y, char character, const uint8_t* font, uint8_t color);
extern void draw_text(uint_fast16_t x, uint_fast16_t y, char* text, const uint8_t* font, uint8_t color);
extern void draw_int(uint_fast16_t x, uint_fast16_t y, int value, uint8_t radix, const uint8_t* font, uint8_t color);
extern void draw_image(uint_fast16_t x, uint_fast16_t y, uint_fast16_t width, uint_fast16_t height, const uint8_t* image);
extern void draw_mouse_cursor(const mouse_state_t* state);
