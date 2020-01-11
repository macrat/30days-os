#pragma once

#include <stdint.h>
#include <stdbool.h>


#define PORT_KEYDAT  0x60
#define PORT_KEYSTA  0x64
#define PORT_KEYCMD  0x64

#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_WRITE_MODE    0x60
#define KEYCMD_SENDTO_MOUSE  0xd4
#define KBC_MODE             0x47
#define MOUSECMD_ENABLE      0xf4


typedef struct BootInfo {
    uint8_t cyls, leds, vmode, reserve;
    uint16_t screen_x, screen_y;
    uint8_t* vram;
} bootinfo_t;

#define get_bootinfo()  ((bootinfo_t*)(0x0ff0))

extern uint8_t io_in8(int port);
extern void io_out8(int port, int data);

extern uint32_t io_load_eflags();
extern void io_store_eflags(uint32_t eflags);

extern void load_idtr(int limit, void* address);
extern void load_gdtr(int limit, void* address);

extern void asm_interrupt20();
extern void asm_interrupt21();
extern void asm_interrupt2c();

extern bool is_available_address(const void* address);
