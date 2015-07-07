CC=gcc
CCFLAGS=-fPIC -c -Wall
LDFLAGS=-shared -Wl,-soname,liblist.so

all: liblist.so
.PHONY: all

liblist.so: malloc.c
	$(CC) $(CCFLAGS) malloc.c
	$(CC) malloc.o $(LDFLAGS) -o liblist.so

clean:
	rm -f *.o liblist.so
.PHONY: clean
