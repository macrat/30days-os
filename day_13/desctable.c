#include <stdint.h>

#include "desctable.h"
#include "hello.h"

#define ADDR_IDT      ((gate_descriptor_t*)0x0026f800)
#define LIMIT_IDT     ((int)0x7ff)
#define ADDR_GDT      ((segment_descriptor_t*)0x00270000)
#define LIMIT_GDT     ((int)0xffff)
#define ADDR_BOTPAK   ((void*)0x00280000)
#define LIMIT_BOTPAK  ((uint32_t)0x0007ffff)
#define AR_DATA32_RW  ((uint16_t)0x4092)
#define AR_CODE32_ER  ((uint16_t)0x409a)
#define AR_INTGATE32  ((uint16_t)0x008e)


typedef struct SegmentDescriptor {
    uint16_t limit_low, base_low;
    uint8_t  base_mid, access_right;
    uint8_t  limit_high, base_high;
} segment_descriptor_t;

typedef struct GateDescriptor {
    uint16_t offset_low, selector;
    uint8_t  dw_count, access_right;
    uint16_t offset_high;
} gate_descriptor_t;

static void set_segmdesc(segment_descriptor_t* sd, uint32_t limit, uint32_t base, uint16_t ar) {
    if (limit > 0xffff) {
        ar |= 0x8000;  // G_bit = 1
        limit /= 0x1000;
    }
    sd->limit_low    = limit & 0xffff;
    sd->base_low     = base & 0xffff;
    sd->base_mid     = (base >> 16) & 0xff;
    sd->access_right = ar & 0xff;
    sd->limit_high   = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
    sd->base_high    = (base >> 24) & 0xff;
}

static void set_gatedesc(gate_descriptor_t* gd, uint32_t offset, uint16_t selector, uint16_t ar) {
	gd->offset_low   = offset & 0xffff;
	gd->selector     = selector;
	gd->dw_count     = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	gd->offset_high  = (offset >> 16) & 0xffff;
}

static void init_gdt() {
    segment_descriptor_t* const gdt = ADDR_GDT;

    for (int i = 0; i <= LIMIT_GDT / 8; i++) {
        set_segmdesc(gdt + i, 0, 0, 0);
    }
    set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, AR_DATA32_RW);
    set_segmdesc(gdt + 2, LIMIT_BOTPAK, (uint32_t)ADDR_BOTPAK, AR_CODE32_ER);

	load_gdtr(LIMIT_GDT, ADDR_GDT);
}

static void init_idt() {
    gate_descriptor_t* const idt = ADDR_IDT;

    for (int i = 0; i <= LIMIT_IDT / 8; i++) {
        set_gatedesc(idt + i, 0, 0, 0);
    }
    load_idtr(LIMIT_IDT, ADDR_IDT);

	set_gatedesc(idt + 0x20, (uint32_t)asm_interrupt20, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x21, (uint32_t)asm_interrupt21, 2 * 8, AR_INTGATE32);
    set_gatedesc(idt + 0x2c, (uint32_t)asm_interrupt2c, 2 * 8, AR_INTGATE32);
}

void init_descriptor_table() {
    init_gdt();
    init_idt();
}
