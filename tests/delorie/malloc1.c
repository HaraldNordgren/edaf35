/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */

#include <stdio.h>

char *buckets[32] = {0};
int bucket2size[32] = {0};

static inline int
size2bucket(int size)
{
  int rv = 0x1f;
  int bit = ~0x10;
  int i;

  if (size < 4) size = 4;
  size = (size+3)&~3;

  for (i=0; i<5; i++)
  {
    if (bucket2size[rv&bit] >= size)
      rv &= bit;
    bit>>=1;
  }
  return rv;
}

static void
init_buckets()
{
  unsigned b;
  for (b=0; b<32; b++)
    bucket2size[b] = (1<<b);
}

char *
test_malloc(int size)
{
  char *rv;
  int b, s, i;

  if (bucket2size[0] == 0)
    init_buckets();

  b = size2bucket(size);
  if (buckets[b])
  {
    rv = buckets[b];
    buckets[b] = *(char **)rv;
    return rv;
  }

  size = bucket2size[b]+4;
  rv = (char *)sbrk(size);

  *(int *)rv = b;
  rv += 4;
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
  int oldsize = bucket2size[*(int *)(ptr-4)];
  if (size <= oldsize)
    return ptr;
  newptr = (char *)test_malloc(size);
  memcpy(ptr, newptr, oldsize);
  test_free(ptr);
  return newptr;

}
