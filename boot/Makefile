.PHONY: all clean

all: sector.bin loader.bin

sector.bin: sector.S
	@echo "Compiling bootsector ..."
	$(AS) $< -o sector.o
	$(LD) -Ttext=0x7c00 --oformat binary -nostdlib -static sector.o -o $@

OBJS = loader_asm.o loader_c.o loader_16bit.o loader_ext2.o \
	   ../dev/ide.o ../dev/console.o \
	   ../asm/asm.o ../lib/lib.o

loader.bin: loader.S loader.c loader_16bit.c loader_ext2.c
	$(CC) $(INCLUDE) $(CFLAGS) loader.S -o loader_asm.o
	$(CC) $(INCLUDE) $(CFLAGS) loader.c -o loader_c.o
	$(CC) $(INCLUDE) $(CFLAGS) loader_16bit.c 
	$(CC) $(INCLUDE) $(CFLAGS) loader_ext2.c
	@echo "Compiling bootloader ..."
	$(CC) -T loader.ld $(OBJS) $(LDFLAGS) -lgcc -o loader.elf
	$(OBJCOPY) -j .text -j .rodata -j .bss -j .data -O binary loader.elf $@

clean:
	rm -f *.o *.bin *.elf
