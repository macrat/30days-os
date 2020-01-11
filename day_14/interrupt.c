#include "events.h"
#include "hello.h"
#include "interrupt.h"

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

#define PIT_CTRL  0x43
#define PIT_CNT0  0x40


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

void init_pit() {
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2f);
}

void enable_interrupt() {
    io_out8(PIC0_IMR, 0xf8);  // enable only from PIC1, keyboard and PIT
    io_out8(PIC1_IMR, 0xef);  // enable only from mouse
}

// INT 21
void keyboard_interrupt() {
    event_t ev = {
        .type     = EVENT_RAW_KEYBOARD,
        .uint8[0] = io_in8(PORT_KEYDAT),
    };

    io_out8(PIC0_OCW2, 0x61);

    push_event(ev);
}

// INT 2c
void mouse_interrupt() {
    event_t ev = {
        .type     = EVENT_RAW_MOUSE,
        .uint8[0] = io_in8(PORT_KEYDAT),
    };

    io_out8(PIC1_OCW2, 0x64);
    io_out8(PIC0_OCW2, 0x62);

    push_event(ev);
}

// INT 60
void timer_interrupt() {
    io_out8(PIC0_OCW2, 0x60);

    static uint32_t counter = 0;
    counter++;

    event_t e = {
        .type      = EVENT_TIMER,
        .uint32[0] = counter,
    };
    push_event(e);
}
