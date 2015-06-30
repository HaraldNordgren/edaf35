CC = gcc
FLAGS = -O0 -W -Wall -Wextra -g

all: liblist.so

liblist.so: alloc.h
	$(CC) $^ $(FLAGS) -o $@ -shared -fPIC

clean:
	rm liblist.so
