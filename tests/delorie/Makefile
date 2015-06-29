# Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details

CC = gcc
CFLAGS = -O3 -g
T = ./$@ > $@.out; ./$@ -r > $@.rout

all : test0 ntest mtest gtest dtest test1 test2 test3 test4 test5 test6
	./plot

ntest : test.o
	$(CC) $(CFLAGS) $^ -o $@
	$T

mtest : test.o
	$(CC) $(CFLAGS) $^ -o $@ -lmalloc
	$T

gtest : test.o gnumalloc/libmalloc.a
	$(CC) $(CFLAGS) $^ -o $@
	$T

dtest : test.o djgppmalloc.o
	$(CC) $(CFLAGS) $^ -o $@
	$T

test0 : test.c malloc0.o
	$(CC) $(CFLAGS) -DTESTMALLOC -DTEST0 $^ -o $@
	$T

test% : ttest.o malloc%.o
	$(CC) $(CFLAGS) $^ -o $@
	$T

ttest.o : test.c
	$(CC) -DTESTMALLOC $(CFLAGS) -c $^ -o $@


clean:
	rm -f *~ [a-z]test *test*.out test[0-9] test0.mtr *.o ideal.out

zip:
	@rm -f mallocs.zip
	zip -9 mallocs.zip Makefile *.c plot
