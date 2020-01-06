#include <stdint.h>

#include "hello.h"
#include "desctable.h"


struct SegmentDescriptor {
    uint16_t limit_low, base_low;
    uint8_t base_mid, access_right;
    uint8_t limit_high, base_high;
};

struct GateDescriptor {
    uint16_t offset_low, selector;
    uint8_t dw_count, access_right;
    uint16_t offset_high;
};

void set_segmdesc(struct SegmentDescriptor *sd, uint32_t limit, uint32_t base, uint16_t ar) {
    if (limit > 0xfffff) {
        ar |= 0x8000;
        limit /= 0x1000;
    }
    sd->limit_low    = limit & 0xffff;
    sd->base_low     = base & 0xffff;
    sd->base_mid     = (base >> 16) & 0xff;
    sd->access_right = ar & 0xff;
    sd->limit_high   = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
    sd->base_high    = (base >> 24) & 0xff;
}

void set_gatedesc(struct GateDescriptor *gd, uint32_t offset, uint16_t selector, uint16_t ar) {
	gd->offset_low   = offset & 0xffff;
	gd->selector     = selector;
	gd->dw_count     = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	gd->offset_high  = (offset >> 16) & 0xffff;
}

void init_gdt() {
    struct SegmentDescriptor *gdt = (struct SegmentDescriptor *) 0x00270000;

    for (int i = 0; i < 8192; i++) {
        set_segmdesc(gdt + i, 0, 0, 0);
    }
    set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092);
    set_segmdesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a);
	load_gdtr(0xffff, 0x00270000);
}

void init_idt() {
    struct GateDescriptor *idt = (struct GateDescriptor *) 0x0026f800;

    for (int i = 0; i < 256; i++) {
        set_gatedesc(idt + i, 0, 0, 0);
    }
    load_idtr(0x7ff, 0x0026f800);
}

void init_descriptor_table() {
    init_gdt();
    init_idt();
}
