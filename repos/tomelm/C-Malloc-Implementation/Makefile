CC = gcc
FLAGS = -O0 -W -Wall -Wextra -g -std=c99

all: liblist.so

liblist.so: my_malloc.c my_sbrk.c
	$(CC) $^ $(FLAGS) -o $@ -shared -fPIC

clean:
	rm liblist.so
