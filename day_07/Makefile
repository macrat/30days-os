.PHONY: build run clean

build: helloos.img

run: helloos.img
	qemu-system-x86_64 -drive file=$^,format=raw,if=floppy

clean:
	-rm *.img *.bin *.o *.sys *.hlo

helloos.img: ipl.bin helloos.sys
	mformat -f 1440 -C -B ipl.bin -i helloos.img ::
	mcopy helloos.sys -i helloos.img ::

helloos.sys: header.bin main.hlo
	cat $^ > $@

main.hlo: hello.ld funcs.o interrupt.c desctable.c graphics.c hankaku.c strings.c events.c main.c
	gcc -march=i486 -m32 -nostdlib -fno-pic -g -T $^ -o $@

.SUFFIXES: .asm .bin .o
.asm.bin:
	nasm $^ -o $@
.asm.o:
	nasm -g -f elf $^ -o $@
