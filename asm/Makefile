SRCS = x86.c
OBJS := $(patsubst %.c, %.o, $(SRCS))

asm.o: $(SRCS)
	for file in $(SRCS); do \
		$(CC) $(INCLUDE) $(CFLAGS) $$file; \
	done
	$(LD) -r $(OBJS) $(LDFLAGS) -o $@

.PHONY: clean
clean:
	rm -f *.o
