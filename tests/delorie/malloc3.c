/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */

#include <stdio.h>

char *buckets[128] = {0};
int bucket2size[128] = {0};

char *pre_brk=0;
int pre_brk_left=0;

static int
size2bucket(int size)
{
  int rv = 0x7f;
  int bit = ~0x40;
  int i;

  if (size < 4) size = 4;
  size = (size+3)&~3;

  for (i=0; i<7; i++)
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
  unsigned b, s, v, sz;
  for (b=0; b<32; b++)
    for (s=0; s<4; s++)
    {
      if (b<8)
        sz = (1U<<b) + ((s<<b)>>2);
      else
	sz = (1U<<b) + (s<<(b-2));
      v = b*4+s;
      if (sz < 4) sz = 4;
      bucket2size[v] = sz;
    }
}

char *
test_malloc(int size)
{
  char *rv;
  int b, s, i;

  if (bucket2size[0] == 0)
    init_buckets();

#if 0
  for (b=0; b<128; b++)
    printf("%3d -> %08x\n", b, bucket2size[b]);
  for (b=0; b<31; b++)
    for (s=1; s<8; s++)
      printf("%08x -> %2d\n", s<<b, size2bucket(s<<b));
#endif

#if 0
  for (s=0; s<66000; s++)
  {
    b = size2bucket(s);
    if (s > bucket2size[b] || (b && (s < bucket2size[b-1]-4)))
    {
      printf("size=%08x b=%3d  r=[%08x-%08x]\n",
	     s, b, bucket2size[b-1], bucket2size[b]);
      exit(0);
    }
  }
  for (i=0; i<2000000; i++)
  {
    s = rand();
    b = size2bucket(s);
    if (s > bucket2size[b] || (b && (s < bucket2size[b-1]-4)))
    {
      printf("size=%08x b=%3d  r=[%08x-%08x]\n",
	     s, b, bucket2size[b-1], bucket2size[b]);
      exit(0);
    }
  }
  exit(0);
#endif

  b = size2bucket(size);
  if (buckets[b])
  {
    rv = buckets[b];
    buckets[b] = *(char **)rv;
    return rv;
  }

  size = bucket2size[b]+4;
  if (pre_brk_left < size)
  {
#define BULK 4095
    int sbs = (size+BULK)&~BULK;
    char *sb = (char *)sbrk(sbs);
    if (pre_brk == 0)
      pre_brk = sb;
    pre_brk_left = (sb+sbs)-pre_brk;
  }

  rv = pre_brk;
  pre_brk += size;
  pre_brk_left -= size;

  *(int *)rv = b;
  rv += 4;
  return rv;
}

test_free(char *ptr)
{
  int b = *(int *)(ptr-4);
  if (pre_brk && ptr + bucket2size[b] >= pre_brk)
  {
    pre_brk_left += (pre_brk - ptr);
    pre_brk = ptr;
    if (pre_brk_left > 4096)
    {
      sbrk(-pre_brk_left);
      pre_brk_left = 0;
    }
  }
  else
  {
    *(char **)ptr = buckets[b];
    buckets[b] = ptr;
  }
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
