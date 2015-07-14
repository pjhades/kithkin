SRCS := setup.c bootmem.c buddy.c slab.c
OBJS := $(patsubst %.c, %.o, $(SRCS))

mm.o: $(SRCS)
	for file in $(SRCS); do \
		$(CC) $(INCLUDE) $(CFLAGS) $$file; \
	done
	$(LD) -r $(OBJS) $(LDFLAGS) -o $@

.PHONY: clean
clean:
	rm -f *.o
