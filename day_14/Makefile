.PHONY: build run clean

CC = gcc
CFLAGS = -Wall -Wextra -Wno-main -fshort-enums -std=c11
C_SOURCES = \
	desctable.c \
	events.c \
	graphics.c \
	hankaku.c \
	input_devices.c \
	interrupt.c \
	memory.c \
	olist.c \
	strings.c \
	timer.c \
	window.c \
	main.c

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

main.hlo: hello.ld funcs.o ${C_SOURCES}
	${CC} ${CFLAGS} -march=i486 -m32 -nostdlib -fno-pic -g -T $^ -o $@

.SUFFIXES: .asm .bin .o
.asm.bin:
	nasm $^ -o $@
.asm.o:
	nasm -g -f elf $^ -o $@
