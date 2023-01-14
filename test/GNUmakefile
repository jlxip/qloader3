include $(CROSS_FILE)

override CC := $(CROSS_CC)
CFLAGS = -O2 -g -Wall -Wextra
LDFLAGS =
override LD := $(CROSS_LD)
override QEMU := qemu-system-x86_64
QEMUFLAGS = -m 1G -enable-kvm -cpu host

override INTERNALLDFLAGS :=         \
	-Tlinker.ld            \
	-nostdlib              \
	-zmax-page-size=0x1000 \
	-static                \
	-pie                   \
	--no-dynamic-linker    \
	-ztext

override INTERNALCFLAGS  :=       \
	-std=c11             \
	-ffreestanding       \
	-fno-stack-protector \
	-fno-stack-check     \
	-fno-lto             \
	-fpie                \
	-mgeneral-regs-only  \
	-mno-red-zone        \
	-I../stivale         \
	-I.                  \
	-I..

all: test.elf

test.elf: stivale2.o e9print.o memory.o
	$(LD) $^ $(LDFLAGS) $(INTERNALLDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INTERNALCFLAGS) -c $< -o $@

%.o: %.asm
	nasm -felf32 -F dwarf -g $< -o $@

clean:
	rm -rf test.elf stivale2.o e9print.o memory.o
