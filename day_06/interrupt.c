#include "hello.h"
#include "interrupt.h"
#include "graphics.h"
#include "hankaku.h"

#define PIC0_ICW1  0x20
#define PIC0_OCW2  0x20
#define PIC0_IMR   0x21
#define PIC0_ICW2  0x21
#define PIC0_ICW3  0x21
#define PIC0_ICW4  0x21
#define PIC1_ICW1  0xa0
#define PIC1_OCW2  0xa0
#define PIC1_IMR   0xa1
#define PIC1_ICW2  0xa1
#define PIC1_ICW3  0xa1
#define PIC1_ICW4  0xa1


void init_pic() {
    // disable interruption
    io_out8(PIC0_IMR, 0xff);
    io_out8(PIC1_IMR, 0xff);

    // settings of PIC0
    io_out8(PIC0_ICW1, 0x11);  // edge triger mode
    io_out8(PIC0_ICW2, 0x20);  // INT20-27
    io_out8(PIC0_ICW3, 1<<2);  // PIC1 is connected to IRQ2
    io_out8(PIC0_ICW4, 0x01);  // non buffer mode

    // settings of PIC1
    io_out8(PIC1_ICW1, 0x11);  // edge triger mode
    io_out8(PIC1_ICW2, 0x28);  // INT28-2f
    io_out8(PIC1_ICW3,    2);  // PIC1 is connected to IRQ2
    io_out8(PIC1_ICW4, 0x01);  // non buffer mode

    // enable interruption
    io_out8(PIC0_IMR, 0xfb);  // enable only from PIC1
    io_out8(PIC1_IMR, 0xff);  // disable all
}

// enable interruption from
void enable_interrupt() {
    io_out8(PIC0_IMR, 0xf9);  // enable only from PIC1 and keyboard
    io_out8(PIC1_IMR, 0xef);  // enable only from mouse
}

// INT 21
void keyboard_interrupt(int *esp) {
    draw_rect(0, 0, 8*17, 16, COLOR_BLACK);
    draw_text(0, 0, "keyboard pressed!", get_hankaku_font(), COLOR_LIGHT_GRAY);

    while (1) {
        io_hlt();
    }
}

// INT 2c
void mouse_interrupt(int *esp) {
    draw_rect(0, 0, 8*12, 16, COLOR_BLACK);
    draw_text(0, 0, "mouse moved!", get_hankaku_font(), COLOR_LIGHT_GRAY);

    while (1) {
        io_hlt();
    }
}
