#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "hello.h"
#include "input_devices.h"
#include "olist.h"


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


typedef struct Color {
    uint8_t R, G, B;
} color_t;

extern const color_t SIMPLE_COLORS[];

typedef struct Sprite {
    int32_t       x, y;
    uint32_t      width, height;
    uint8_t*      image;
    olist_item_t* olist_item;
} sprite_t;

extern void init_screen();
extern void refresh_screen();

#define get_screen_width()   (get_bootinfo()->screen_x)
#define get_screen_height()  (get_bootinfo()->screen_y)

extern sprite_t* create_sprite(int32_t x, int32_t y, uint32_t width, uint32_t height, uint8_t order);
extern void destroy_sprite(sprite_t* sprite);
extern void move_sprite(sprite_t* sprite, int32_t x, int32_t y, uint8_t order);
extern int count_sprites();

extern void set_color(uint8_t id, const color_t color);
extern void set_palette(const color_t colors[]);

extern void fill_sprite(sprite_t* sprite, uint8_t color);
extern void draw_pixel(sprite_t* sprite, int32_t x, int32_t y, uint8_t color);
extern void draw_rect(sprite_t* sprite, int32_t x, int32_t y, uint32_t width, uint32_t height, uint8_t color);
extern void draw_char(sprite_t* sprite, int32_t x, int32_t y, char character, const uint8_t* font, uint8_t fg, uint8_t bg);
extern void draw_text(sprite_t* sprite, int32_t x, int32_t y, char* text, const uint8_t* font, uint8_t fg, uint8_t bg);
extern void draw_int(sprite_t* sprite, int32_t x, int32_t y, int value, uint8_t radix, const uint8_t* font, uint8_t fg, uint8_t bg);
extern void draw_image(sprite_t* sprite, int32_t x, int32_t y, uint32_t width, uint32_t height, const uint8_t* image);

extern sprite_t* create_cursor_sprite();
extern void move_cursor_sprite(sprite_t* sprite, const mouse_state_t* state);
