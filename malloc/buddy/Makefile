CC			= gcc
CFLAGS		= -g -Wall -Wextra -pedantic -std=c11 -m64

# Targets
PROGS =	test test2 libbuddy.so

all: clean $(PROGS)

alloc.o: alloc.c
	$(CC) $(CFLAGS) -c -fPIC $^ -o $@

test: alloc.o test.c
	$(CC) $(CFLAGS) $^ -o $@

test2: alloc.o test2.c
	$(CC) $(CFLAGS) $^ -o $@

libbuddy.so: alloc.o
	$(CC) $(CFLAGS) $^ -shared -o $@

# Standard clean
clean:
	rm -f *.o $(PROGS)
