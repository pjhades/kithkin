SRCS = console.c ide.c 

.PHONY: all clean

all: 
	for file in $(SRCS); do \
		$(CC) $(INCLUDE) $(CFLAGS) $$file; \
	done

clean:
	rm -f *.o
