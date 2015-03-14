INC = -I./include
CFLAGS = -ffreestanding -m32 -c

DEPS_SRCS := $(wildcard arch/*.c driver/*.c fs/*.c lib/*.c)
DEPS_OBJS := $(patsubst %.c, %.o, $(DEPS_SRCS))

KERNEL_SRCS := $(wildcard kernel/*.c kernel/*.S)
KERNEL_OBJS := $(patsubst %.c, %.o, $(patsubst %.S, %.o, $(KERNEL_SRCS)))

LOADER_OBJ_FILES = boot/loader_asm.o \
				   boot/loader_c.o \
				   boot/loader_c16.o \
				   boot/loader_ext2.o \
				   arch/x86.o \
				   driver/ide.o \
				   driver/console.o \
				   lib/string.o

disk_image: boot/sector.bin boot/loader.bin kernel/kernel.img
	@echo ">>> Making the disk image ..."
	sudo losetup -o $$((512*2048)) /dev/loop0 disk.img
	sudo mount /dev/loop0 /mnt/test
	sudo cp kernel/kernel.img /mnt/test/boot/kernel.img
	sudo umount /mnt/test
	sudo losetup -d /dev/loop0
	dd conv=notrunc if=boot/sector.bin of=disk.img bs=446 count=1
	dd conv=notrunc skip=510 seek=510 if=boot/sector.bin of=disk.img bs=1 count=2
	dd conv=notrunc seek=1 if=boot/loader.bin of=disk.img bs=512 \
		count=$$(( ($$(stat --format '%s' boot/loader.bin) + 511)/ 512 ))
	sudo cp disk.img /media/sf_pjhades/code/lab

boot/sector.bin: boot/sector.S
	@echo ">>> Compiling bootsector code ..."
	as $< -o boot/sector.o
	ld -Ttext=0x7c00 --oformat binary -nostdlib -static boot/sector.o -o $@

boot/loader_asm.o: boot/loader.S boot/loader.c boot/loader16bit.c
	gcc $(INC) $(CFLAGS) boot/loader.S -o boot/loader_asm.o
	gcc $(INC) $(CFLAGS) boot/loader.c -o boot/loader_c.o
	gcc $(INC) $(CFLAGS) boot/loader16bit.c -o boot/loader_c16.o
	gcc $(INC) $(CFLAGS) boot/loader_ext2.c -o boot/loader_ext2.o

boot/loader.bin: deps boot/loader_asm.o
	@echo ">>> Compiling bootloader ..."
	ld -T boot/loader.ld -m elf_i386 $(LOADER_OBJ_FILES) -o boot/loader.elf
	objcopy -j .text -j .rodata -j .bss -j .data -O binary boot/loader.elf $@

.PHONY: deps
deps:
	@echo ">>> Compiling dependencies ..."
	for file in $(DEPS_SRCS); do \
		gcc $(INC) $(CFLAGS) $$file -o $${file%.c}.o; \
	done

kernel/kernel.img: $(KERNEL_SRCS) $(DEPS_OBJS)
	@echo ">>> Compiling kernel ..."
	for file in $(KERNEL_SRCS); do \
		if [[ "$$file" =~ \.S$$ ]]; then \
			gcc $(INC) $(CFLAGS) $$file -o $${file%.S}.o; \
		else \
			gcc $(INC) $(CFLAGS) $$file -o $${file%.c}.o; \
		fi; \
	done
	ld -T kernel/kernel.ld -m elf_i386 $(KERNEL_OBJS) $(DEPS_OBJS) -o $@

.PHONY: clean
clean:
	rm -f boot/*.{o,bin,elf}
	rm -f arch/*.o
	rm -f fs/*.o
	rm -f lib/*.o
	rm -f driver/*.o
	rm -f kernel/*.{o,img}
