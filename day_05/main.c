#include <stdint.h>

#include "hello.h"
#include "graphics.h"
#include "hankaku.h"
#include "desctable.h"


bootinfo_t *get_bootinfo() {
    return (bootinfo_t*)(0x0ff0);
}

void main() {
    init_descriptor_table();

    set_palette(SIMPLE_COLORS);

    draw_rect(0, 0, get_screen_x(), get_screen_y(), COLOR_WHITE);
    draw_rect(20, 20, 30, 30, COLOR_RED);

    draw_text(55, 20, "012345678901234567890\nhello\tworld!\nthis\tis\ta\ttest", get_hankaku_font(), COLOR_RED);

    draw_mouse_cursor(20, 60);

    while (1) {
        io_hlt();
    }
}
