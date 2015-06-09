SRCS = entry.S main.c
OBJS := $(patsubst %.c, %.o, $(patsubst %.S, %.o, $(SRCS)))

DEPOBJS = ../dev/console.o ../lib/lib.o ../asm/asm.o

kernel.img: $(SRCS)
	@echo "Compiling kernel ..."	
	for file in $(SRCS); do \
		$(CC) $(INCLUDE) $(CFLAGS) $$file; \
	done
	$(CC) -T kernel.ld $(OBJS) $(DEPOBJS) $(LDFLAGS) -lgcc -o $@

.PHONY: clean
clean:
	rm -f *.o *.img
