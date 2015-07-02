CC=gcc
CCFLAGS=-fPIC -c -Wall
LDFLAGS=-shared -Wl,-soname,libmalloc.so

all: libmalloc.so
.PHONY: all

libmalloc.so: buddy.c
	$(CC) $(CCFLAGS) buddy.c
	$(CC) buddy.o $(LDFLAGS) -o libmalloc.so

#test: test.c
#	gcc -o test test.c

clean:
	rm -f *.o test libmalloc.so
.PHONY: clean
