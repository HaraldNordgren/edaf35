CC			= gcc
#CFLAGS		= -g -Wall
LDFLAGS		= -g

# Targets
PROGS =	alloc #liblist.so

all:	$(PROGS)

liblist.so: alloc.c
	$(CC) $^ $(LDFLAGS) -o $@ -shared -fPIC

# Standard clean
clean:
	rm -f *.o $(PROGS)

