.PHONY: all clean

all: sector.bin loader.bin

sector.bin: sector.S
	@echo "Compiling bootsector ..."
	$(AS) $< -o sector.o
	$(LD) -Ttext=0x7c00 --oformat binary -nostdlib -static sector.o -o $@

OBJS = loader_asm.o loader_c.o loader_16bit.o loader_ext2.o \
	   ../driver/ide.o ../driver/console.o \
	   ../asm/asm.o   ../lib/string.o

loader.bin: loader.S loader.c loader_16bit.c loader_ext2.c
	$(CC) $(INCLUDE) $(CFLAGS) loader.S -o loader_asm.o
	$(CC) $(INCLUDE) $(CFLAGS) loader.c -o loader_c.o
	$(CC) $(INCLUDE) $(CFLAGS) loader_16bit.c 
	$(CC) $(INCLUDE) $(CFLAGS) loader_ext2.c
	@echo "Compiling bootloader ..."
	$(LD) -T loader.ld -m elf_i386 $(OBJS) -o loader.elf
	objcopy -j .text -j .rodata -j .bss -j .data -O binary loader.elf $@

clean:
	rm -f *.o *.bin *.elf
