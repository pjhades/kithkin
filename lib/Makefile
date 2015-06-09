SRCS = printk.c string.c 
OBJS := $(patsubst %.c, %.o, $(SRCS))

lib.o: $(SRCS)
	for file in $(SRCS); do \
		$(CC) $(INCLUDE) $(CFLAGS) $$file; \
	done
	$(LD) -r $(OBJS) $(LDFLAGS) -o $@

.PHONY: clean
clean:
	rm -f *.o
