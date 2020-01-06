#include <stdint.h>

#include "desctable.h"
#include "graphics.h"
#include "hankaku.h"
#include "hello.h"
#include "interrupt.h"


bootinfo_t *get_bootinfo() {
    return (bootinfo_t*)(0x0ff0);
}

void main() {
    init_descriptor_table();
    init_pic();
    io_sti();

    set_palette(SIMPLE_COLORS);

    draw_rect(0, 0, get_screen_x(), get_screen_y(), COLOR_WHITE);
    draw_rect(20, 20, 30, 30, COLOR_RED);

    draw_text(55, 20, "012345678901234567890\nhello\tworld!\nthis\tis\ta\ttest", get_hankaku_font(), COLOR_RED);

    draw_mouse_cursor(20, 60);

    enable_interrupt();

    /*
    while (1) {
        io_hlt();
    }
    */
}
