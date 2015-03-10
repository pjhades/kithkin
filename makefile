INC = -I./include
CFLAGS = -ffreestanding -m32 -c

.PHONY: fs arch driver lib

disk_image: boot/sector.bin boot/loader.bin kernel/kernel.img
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
	as $< -o boot/sector.o
	ld -Ttext=0x7c00 --oformat binary -nostdlib -static boot/sector.o -o $@

boot/loader.bin: boot/loader.S boot/loader.c boot/loader16bit.c \
	         arch/x86.o driver/console.o driver/ide.o lib/string.o \
			 fs/ext2_loader.o
	gcc $(INC) $(CFLAGS) boot/loader.S -o boot/loader_asm.o
	gcc $(INC) $(CFLAGS) boot/loader.c -o boot/loader_c.o
	gcc $(INC) $(CFLAGS) boot/loader16bit.c -o boot/loader_c16.o
	ld -T boot/loader.ld -m elf_i386 \
		boot/loader_asm.o \
		boot/loader_c.o \
		boot/loader_c16.o \
		arch/x86.o \
		driver/ide.o \
		driver/console.o \
		fs/ext2_loader.o \
		lib/string.o \
		-o boot/loader.elf
	objcopy -j .text -j .rodata -j .bss -j .data -O binary boot/loader.elf $@

fs/ext2_loader.o: fs/ext2_loader.c
	gcc $(INC) $(CFLAGS) $< -o $@

arch/x86.o: arch/x86.c
	gcc $(INC) $(CFLAGS) $< -o $@

driver/console.o: driver/console.c
	gcc $(INC) $(CFLAGS) $< -o $@

driver/ide.o: driver/ide.c
	gcc $(INC) $(CFLAGS) $< -o $@

lib/string.o: lib/string.c
	gcc $(INC) $(CFLAGS) $< -o $@

kernel/kernel.img: kernel/entry.S kernel/main.c driver/console.o \
	               arch/x86.o
	gcc $(INC) $(CFLAGS) $< -o kernel/entry.o
	gcc $(INC) $(CFLAGS) kernel/main.c -o kernel/main.o
	ld -T kernel/kernel.ld -m elf_i386 \
		kernel/entry.o \
		kernel/main.o \
		arch/x86.o \
		driver/console.o \
		-o $@

.PHONY: clean
clean:
	rm -f boot/*.{o,bin,elf}
	rm -f arch/*.o
	rm -f fs/*.o
	rm -f lib/*.o
	rm -f driver/*.o
	rm -f kernel/*.{o,img}
