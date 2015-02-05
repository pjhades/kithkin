BOOTSEC_NAME = bootsector
BOOTLDR_NAME = bootloader

img: bootsec bootldr
	rm -f disk.img
	cp backup.img disk.img
	dd conv=notrunc if=$(BOOTSEC_NAME).bin of=disk.img bs=446 count=1
	dd conv=notrunc skip=510 seek=510 if=$(BOOTSEC_NAME).bin of=disk.img bs=1 count=2
	dd conv=notrunc seek=1 if=$(BOOTLDR_NAME).bin of=disk.img bs=512 count=1
	sudo cp disk.img /media/sf_pjhades/code/lab

bootsec: $(BOOTSEC_NAME).s
	as $< -o $(BOOTSEC_NAME).o
	ld -Ttext=0x7c00 --oformat binary -nostdlib -static $(BOOTSEC_NAME).o -o $(BOOTSEC_NAME).bin

bootldr: $(BOOTLDR_NAME).s
	as $(BOOTLDR_NAME).s -o $(BOOTLDR_NAME).o
	ld -Ttext=0x0500 --oformat binary -nostdlib -static $(BOOTLDR_NAME).o -o $(BOOTLDR_NAME).bin

.PHONY: clean
clean:
	rm -f *.o *.bin
