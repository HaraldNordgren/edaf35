CC			= gcc
CFLAGS		= -g -Wall -Wextra -std=c11 -m64

# Targets
PROGS =	test liblist.so

all: clean $(PROGS)

alloc.o: alloc.c
	$(CC) $(CFLAGS) -c -fPIC $^ -o $@

test: alloc.o test.c
	$(CC) $(CFLAGS) $^ -o $@

liblist.so: alloc.o
	$(CC) $^ $(CFLAGS) -shared -o $@

# Standard clean
clean:
	rm -f *.o $(PROGS)
