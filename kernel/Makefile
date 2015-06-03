SRCS = entry.S main.c
OBJS := $(patsubst %.c, %.o, $(patsubst %.S, %.o, $(SRCS)))

DEPOBJS = ../driver/console.o ../lib/lib.o ../asm/asm.o

kernel.img: $(SRCS)
	@echo "Compiling kernel ..."	
	for file in $(SRCS); do \
		$(CC) $(INCLUDE) $(CFLAGS) $$file; \
	done
	ld -T kernel.ld -m elf_i386 $(OBJS) $(DEPOBJS) -o $@

.PHONY: clean
clean:
	rm -f *.o *.img
