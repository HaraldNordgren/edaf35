CC		= gcc
CFLAGS	= -Wall -pedantic -std=c99
OUT		= buddy_test
SOURCES	= buddy.c test.c
OBJECTS	= $(SOURCES:.c=.o)

all: $(OBJECTS)

.c.o: $(SOURCES)
	$(CC) $(CFLAGS) $< -c -g -o $@

$(OUT): $(OBJECTS)
	$(CC) -g -o $(OUT) $(OBJECTS)

test: $(OUT)
	./$(OUT)

clean:
	rm -f $(OBJECTS)
