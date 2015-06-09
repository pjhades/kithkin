PREFIX := $(HOME)/cross/bin/i686-elf-

export CC      := $(PREFIX)gcc
export AS      := $(PREFIX)as
export LD      := $(PREFIX)ld
export OBJCOPY := $(PREFIX)objcopy
export INCLUDE := -I$(shell pwd)/include
export CFLAGS  := -Wall -ffreestanding -c
export LDFLAGS := -nostdlib

DEPS := asm lib dev boot kernel

.PHONY: all subdirs image

all: subdirs image

subdirs:
	for dir in $(DEPS); do \
		(cd $$dir; $(MAKE)); \
	done

image: boot/sector.bin boot/loader.bin kernel/kernel.img
	@echo "Making the disk image ..."
	sudo losetup -o $$((512*2048)) /dev/loop0 disk.img
	sudo mount /dev/loop0 /mnt/test
	sudo cp kernel/kernel.img /mnt/test/boot/kernel.img
	sudo umount /mnt/test
	sudo losetup -d /dev/loop0
	dd conv=notrunc if=boot/sector.bin of=disk.img bs=446 count=1
	dd conv=notrunc skip=510 seek=510 if=boot/sector.bin of=disk.img bs=1 count=2
	dd conv=notrunc seek=1 if=boot/loader.bin of=disk.img bs=512 count=$$(( ($$(stat --format '%s' boot/loader.bin) + 511)/ 512 ))
	sudo cp disk.img /media/sf_pjhades/code/lab

.PHONY: clean
clean:
	for dir in $(DEPS); do \
		(cd $$dir; $(MAKE) clean); \
	done
