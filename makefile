INC = -I. -I./include

.PHONY: fs kernel arch driver lib

image: bootsec bootldr
	dd conv=notrunc if=boot/sec.bin of=disk.img bs=446 count=1
	dd conv=notrunc skip=510 seek=510 if=boot/sec.bin of=disk.img bs=1 count=2
	dd conv=notrunc seek=1 if=boot/ldr.bin of=disk.img bs=512 \
		count=$$(( ($$(stat --format '%s' boot/ldr.bin) + 511)/ 512 ))
	sudo cp disk.img /media/sf_pjhades/code/lab

bootsec: boot/sector.S
	as $< -o boot/sec.o
	ld -Ttext=0x7c00 --oformat binary -nostdlib -static boot/sec.o -o boot/sec.bin

bootldr: boot/loader.S boot/loader.c arch driver fs lib
	gcc $(INC) -ffreestanding -m32 -c boot/loader.S -o boot/ldr_asm.o
	gcc $(INC) -ffreestanding -m32 -c boot/loader.c -o boot/ldr_c.o
	ld -T boot/loader.ld -m elf_i386 \
		boot/ldr_asm.o \
		boot/ldr_c.o \
		arch/x86.o \
		driver/ide.o \
		driver/tty.o \
		fs/ext2.o \
		lib/lib.o \
		-o boot/ldr.elf
	objcopy -j .text -j .rodata -j .bss -j .data -O binary boot/ldr.elf boot/ldr.bin

fs:
	gcc $(INC) -ffreestanding -m32 -c fs/*.c -o fs/ext2.o

arch:
	gcc $(INC) -ffreestanding -m32 -c arch/x86.c -o arch/x86.o

driver:
	gcc $(INC) -ffreestanding -m32 -c driver/ide.c -o driver/ide.o
	gcc $(INC) -ffreestanding -m32 -c driver/tty.c -o driver/tty.o

kernel:
	gcc $(INC) -ffreestanding -m32 -c kernel/entry.S -o kernel/kernel.o
	ld -T kernel/kernel.ld -m elf_i386 kernel/kernel.o -o kernel/kernel.img
	sudo losetup -o $$((512*2048)) /dev/loop0 disk.img
	sudo mount /dev/loop0 /mnt/test
	sudo cp kernel/kernel.img /mnt/test/boot/kernel.img
	sudo umount /mnt/test
	sudo losetup -d /dev/loop0

lib:
	gcc $(INC) -ffreestanding -m32 -c lib/*.c -o lib/lib.o

.PHONY: clean
clean:
	rm -f boot/*.{o,bin,elf}
	rm -f arch/*.o
	rm -f kernel/*.o
