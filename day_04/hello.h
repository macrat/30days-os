#pragma once

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
