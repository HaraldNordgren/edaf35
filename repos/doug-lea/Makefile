CC = clang
FLAGS = -O0 -W -Wall -Wextra -g

all: liblist.so

liblist.so: malloc.c
	$(CC) $^ $(FLAGS) -o $@ -shared -fPIC

