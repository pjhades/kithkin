SRCS = printk.c string.c 
OBJS := $(patsubst %.c, %.o, $(SRCS))

lib.o: $(SRCS)
	for file in $(SRCS); do \
		$(CC) $(INCLUDE) $(CFLAGS) $$file; \
	done
	$(LD) -r -m elf_i386 $(OBJS) -o $@

.PHONY: clean
clean:
	rm -f *.o
