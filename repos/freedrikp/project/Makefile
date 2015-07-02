CC = clang
FLAGS = -O0 -W -Wall -Wextra -g

all: liblist.so

liblist.so: malloclist.c
	$(CC) $^ $(FLAGS) -o $@ -shared -fPIC
