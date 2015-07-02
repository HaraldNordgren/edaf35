# Makefile template for project 3

CC = gcc
CFLAGS = -g -Wall
# main executable file first
TARGET = proj03
# object files next
OBJS = main.o buddy.o
# header files next
DEPS = 
.PHONY : clean

<<<<<<< HEAD
all: $(TARGET) libbuddy.so

libbuddy.so: buddy.c
	$(CC) $^ $(FLAGS) -o $@ -shared -fPIC
=======
all: $(TARGET)
>>>>>>> eb6aac9fda577624bedb926aca10ca5da58ce64f

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

.c.o: $(DEPS)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET) *~

