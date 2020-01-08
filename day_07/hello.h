#pragma once

#include <stdint.h>


#define PORT_KEYDAT  0x60
#define PORT_KEYSTA  0x64
#define PORT_KEYCMD  0x64

#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_WRITE_MODE    0x60
#define KEYCMD_SENDTO_MOUSE  0xd4
#define KBC_MODE             0x47
#define MOUSECMD_ENABLE      0xf4


struct BootInfo {
    uint8_t cyls, leds, vmode, reserve;
    uint16_t screen_x, screen_y;
    uint8_t *vram;
} typedef bootinfo_t;

extern bootinfo_t *get_bootinfo();

extern void io_hlt();
extern void io_cli();
extern void io_sti();

extern int io_in8(int port);
extern int io_in16(int port);
extern int io_in32(int port);

extern void io_out8(int port, int data);
extern void io_out16(int port, int data);
extern void io_out32(int port, int data);

extern int io_load_eflags();
extern void io_store_eflags(int eflags);

extern void load_idtr(int limit, void* address);
extern void load_gdtr(int limit, void* address);

extern void asm_interrupt21();
extern void asm_interrupt2c();
