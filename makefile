INC = -I./include
CFLAGS = -ffreestanding -m32 -c

.PHONY: fs arch driver lib

image: bootsec bootldr
	dd conv=notrunc if=boot/sec.bin of=disk.img bs=446 count=1
	dd conv=notrunc skip=510 seek=510 if=boot/sec.bin of=disk.img bs=1 count=2
	dd conv=notrunc seek=1 if=boot/loadr.bin of=disk.img bs=512 \
		count=$$(( ($$(stat --format '%s' boot/loadr.bin) + 511)/ 512 ))
	sudo cp disk.img /media/sf_pjhades/code/lab

bootsec: boot/sector.S
	as $< -o boot/sec.o
	ld -Ttext=0x7c00 --oformat binary -nostdlib -static boot/sec.o -o boot/sec.bin

bootldr: boot/loader.S boot/loader.c boot/loader16bit.c arch driver fs lib
	gcc $(INC) $(CFLAGS) boot/loader.S -o boot/loadr_asm.o
	gcc $(INC) $(CFLAGS) boot/loader.c -o boot/loadr_c.o
	gcc $(INC) $(CFLAGS) boot/loader16bit.c -o boot/loadr_c16.o
	ld -T boot/loader.ld -m elf_i386 \
		boot/loadr_asm.o \
		boot/loadr_c.o \
		boot/loadr_c16.o \
		arch/x86.o \
		driver/ide.o \
		driver/console.o \
		fs/ext2.o \
		lib/lib.o \
		-o boot/loadr.elf
	objcopy -j .text -j .rodata -j .bss -j .data -O binary boot/loadr.elf boot/loadr.bin

fs:
	gcc $(INC) $(CFLAGS) fs/*.c -o fs/ext2.o

arch:
	gcc $(INC) $(CFLAGS) arch/x86.c -o arch/x86.o

driver:
	gcc $(INC) $(CFLAGS) driver/ide.c -o driver/ide.o
	gcc $(INC) $(CFLAGS) driver/console.c -o driver/console.o

kernel: arch driver fs lib
	gcc $(INC) $(CFLAGS) kernel/entry.S -o kernel/kernel.o
	gcc $(INC) $(CFLAGS) kernel/main.c -o kernel/main.o
	ld -T kernel/kernel.ld -m elf_i386 \
		kernel/kernel.o \
		kernel/main.o \
		arch/x86.o \
		driver/console.o \
		-o kernel/kernel.img
	sudo losetup -o $$((512*2048)) /dev/loop0 disk.img
	sudo mount /dev/loop0 /mnt/test
	sudo cp kernel/kernel.img /mnt/test/boot/kernel.img
	sudo umount /mnt/test
	sudo losetup -d /dev/loop0

lib:
	gcc $(INC) $(CFLAGS) lib/*.c -o lib/lib.o

.PHONY: clean
clean:
	rm -f boot/*.{o,bin,elf}
	rm -f arch/*.o
	rm -f kernel/*.{o,img}
