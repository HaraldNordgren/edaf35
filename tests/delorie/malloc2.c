/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */

#include <stdio.h>

#define BULK 4095

char *buckets[32] = {0};

char *pre_brk=0;
int pre_brk_left=0;

int
dumpstats()
{
  int i, count;
  char *rv;
  for (i=0; i<32; i++)
  {
    count=0;
    for (rv=buckets[i]; rv; rv=*(char **)rv) count++;
    if (count)
      fprintf(stderr, "%2d  %12d  %12d\n", i-3, count, (1<<i)*count);
  }
}

char *
test_malloc(int size)
{
  char *rv;
  int b, s, i;

  if (pre_brk == 0)
    atexit(dumpstats);

  for (b=3, s=8; s&&s<size+4; b++, s<<=1);
  if (!s) return 0;
  /*printf("b=%2d size=%08x s=%08x\n", b, size, s);*/

  if (!buckets[b])
  {
    size = (1<<b);
    /*printf("size now %08x\n", size);*/
    if (pre_brk_left < size)
    {
      int sbs = (size+BULK)&~BULK;
      char *sb = (char *)sbrk(sbs);
      if (pre_brk == 0)
	pre_brk = sb;
      pre_brk_left = (sb+sbs)-pre_brk;
    }

    while (pre_brk_left >= size)
    {
      rv = pre_brk;
      pre_brk += size;
      pre_brk_left -= size;

      *(int *)rv = b;
      rv += 4;
      *(char **)rv = buckets[b];
      buckets[b] = rv;
    }
  }
  rv = buckets[b];
  buckets[b] = *(char **)rv;
  return rv;
}

test_free(char *ptr)
{
  int b = *(int *)(ptr-4);
  *(char **)ptr = buckets[b];
  buckets[b] = ptr;
}

char *
test_realloc(char *ptr, int size)
{
  char *newptr;
  int oldsize = 1 << *(int *)(ptr-4);
  if (size <= oldsize)
    return ptr;
  newptr = (char *)test_malloc(size);
  memcpy(ptr, newptr, oldsize);
  test_free(ptr);
  return newptr;
}
