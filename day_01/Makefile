.PHONY: run

run: helloos.img
	qemu-system-x86_64 -drive file=$<,format=raw,if=floppy

helloos.img: helloos.asm
	nasm $^ -o $@
