AS := as
LD := ld
CC := gcc
ASFLAGS := --64
LDFLAGS := -m elf_x86_64 -Ttext 0x0 -s --oformat binary
CFLAGS := -std=c11 -O2 -Wall -Wextra -Werror

.PHONY: image clean
.DELETE_ON_ERROR:

image: linux.img

linux.img: tools/build bootsect setup
	./tools/build bootsect setup > $@

tools/build: tools/build.c
	$(CC) $(CFLAGS) -o $@ $<

bootsect: bootsect.o
	$(LD) $(LDFLAGS) -e _start -o $@ $<

bootsect.o: bootsect.S
	$(AS) $(ASFLAGS) -o $@ $<

setup: setup.o
	$(LD) $(LDFLAGS) -e _start_setup -o $@ $<

setup.o: setup.S
	$(AS) $(ASFLAGS) -o $@ $<

clean:
	$(RM) bootsect.o setup.o bootsect setup tools/build linux.img
