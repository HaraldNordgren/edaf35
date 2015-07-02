CC = gcc
FLAGS = -O0 -W -Wall -Wextra -g

all: liblist.so

liblist.so: free.c malloc.c realloc.c calloc.c get_heap.c fusion.c show_alloc_mem.c
	$(CC) $^ $(FLAGS) -o $@ -shared -fPIC

clean:
	rm liblist.so
