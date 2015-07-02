CC = gcc
FLAGS = -O0 -W -Wall -Wextra -g

all: liblist.so

liblist.so: mem_management.cpp
	$(CC) $^ $(FLAGS) -o $@ -shared -fPIC
