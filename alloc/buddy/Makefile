CC=gcc
CCFLAGS=-fPIC -c -Wall
LDFLAGS=-shared -Wl,-soname,libbuddy.so

all: libbuddy.so
.PHONY: all

libbuddy.so: malloc.c
	$(CC) $(CCFLAGS) malloc.c
	$(CC) malloc.o $(LDFLAGS) -o libbuddy.so

clean:
	rm -f *.o libbuddy.so
.PHONY: clean
