#include <stdint.h>

#include "hello.h"
#include "graphics.h"


void main() {
    set_palette(SIMPLE_COLORS);

    draw_rect(0, 0, get_screen_x(), get_screen_y(), COLOR_WHITE);
    draw_rect(20, 20, 30, 30, COLOR_RED);

    while (1) {
        io_hlt();
    }
}
