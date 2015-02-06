boot_dir = boot
image_dir = image
arch_dir = arch

bootsec_name = bootsector
bootldr_name = bootloader

image: bootsec bootldr
	rm -f $(image_dir)/disk.img
	cp $(image_dir)/backup.img $(image_dir)/disk.img
	dd conv=notrunc if=$(boot_dir)/sec.bin of=$(image_dir)/disk.img bs=446 count=1
	dd conv=notrunc skip=510 seek=510 if=$(boot_dir)/sec.bin of=$(image_dir)/disk.img bs=1 count=2
	dd conv=notrunc seek=1 if=$(boot_dir)/ldr.bin of=$(image_dir)/disk.img bs=512 \
		count=$$(( ($$(stat --format '%s' $(boot_dir)/ldr.bin) + 511)/ 512 ))
	sudo cp $(image_dir)/disk.img /media/sf_pjhades/code/lab

bootsec: $(boot_dir)/$(bootsec_name).s
	as $< -o $(boot_dir)/sec.o
	ld -Ttext=0x7c00 --oformat binary -nostdlib -static $(boot_dir)/sec.o -o $(boot_dir)/sec.bin

bootldr: $(boot_dir)/$(bootldr_name).s $(boot_dir)/$(bootldr_name).c
	as --32 $(boot_dir)/$(bootldr_name).s -o $(boot_dir)/ldr_asm.o
	gcc -I. -nostdinc -m32 -c $(boot_dir)/$(bootldr_name).c -o $(boot_dir)/ldr_c.o
	gcc -I. -nostdinc -m32 -c $(arch_dir)/x86.c -o $(arch_dir)/x86.o
	ld -Ttext 0x0500 -m elf_i386 $(boot_dir)/ldr_asm.o $(boot_dir)/ldr_c.o $(arch_dir)/x86.o -o $(boot_dir)/ldr.elf
	objcopy -j .text -j .rodata -O binary $(boot_dir)/ldr.elf $(boot_dir)/ldr.bin

.PHONY: clean
clean:
	rm -f $(boot_dir)/*.{o,bin,elf}
	rm -f $(arch_dir)/*.o
