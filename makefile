INC = -I.

image: bootsec bootldr
	rm -f disk.img
	cp backup.img disk.img
	dd conv=notrunc if=boot/sec.bin of=disk.img bs=446 count=1
	dd conv=notrunc skip=510 seek=510 if=boot/sec.bin of=disk.img bs=1 count=2
	dd conv=notrunc seek=1 if=boot/ldr.bin of=disk.img bs=512 \
		count=$$(( ($$(stat --format '%s' boot/ldr.bin) + 511)/ 512 ))
	sudo cp disk.img /media/sf_pjhades/code/lab

bootsec: boot/bootsector.s
	as $< -o boot/sec.o
	ld -Ttext=0x7c00 --oformat binary -nostdlib -static boot/sec.o -o boot/sec.bin

bootldr: boot/bootloader.s boot/bootloader.c arch/x86.o
	as --32 boot/bootloader.s -o boot/ldr_asm.o
	gcc $(INC) -nostdinc -m32 -c boot/bootloader.c -o boot/ldr_c.o
	ld -T boot/linker.ld -m elf_i386 boot/ldr_asm.o boot/ldr_c.o arch/x86.o -o boot/ldr.elf
	objcopy -j .text -j .rodata -j .bss -j .data -O binary boot/ldr.elf boot/ldr.bin

arch/x86.o: arch/x86.c
	gcc $(INC) -nostdinc -m32 -c $< -o $@

.PHONY: clean
clean:
	rm -f boot/*.{o,bin,elf}
	rm -f arch/*.o
