CC			= gcc
CFLAGS		= -g -Wall -Wextra -std=c11

# Targets
PROGS =	test libbuddy.so

all:	$(PROGS)

alloc.o: alloc.c
	$(CC) $(CFLAGS) -c -fPIC $^ -o $@

test: alloc.o test.c
	$(CC) $(CFLAGS) $^ -o $@

libbuddy.so: alloc.o
	$(CC) $^ $(CFLAGS) -shared -o $@

# Standard clean
clean:
	rm -f *.o $(PROGS)
