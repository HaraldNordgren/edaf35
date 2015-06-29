/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */

#include <stdio.h>
#include <assert.h>

typedef struct BLOCK {
  int size;
  struct BLOCK *next;
} BLOCK;
#define BEFORE(bp)	((BLOCK *)((char *)bp - *(int *)((char *)bp - 4) - 8))
#define BEFSZ(bp)	(*(int *)((char *)bp - 4))
#define ENDSZ(bp)	(*(int *)((char *)bp + bp->size + 4))
#define AFTER(bp)	((BLOCK *)((char *)bp + bp->size + 8))
#define DATA(bp)	((char *)&(bp->next))

BLOCK *sbrk_block = 0;
BLOCK *slop = 0;
BLOCK *freelist[30];

#define MIN_SAVE_EXTRA	16
#define SBRK_CHUNK	65536
#define BIG_BLOCK	4096

#define DEBUG 0

#if DEBUG
static void
check(BLOCK *b)
{
  printf("check %08x %d %08x %d\n", b, b->size, &(ENDSZ(b)), ENDSZ(b));
}
#define CHECK(p) do { check(p); assert(p->size == ENDSZ(p)); consistency(); } while (0)
#define CHECK1(p) do { check(p); assert(p->size == ENDSZ(p)); } while (0)
static void
consistency()
{
#if 0
  int b;
  BLOCK *bl;
  if (sbrk_block)
    CHECK1(sbrk_block);
  if (slop)
    CHECK1(slop);
  for (b=0; b<32; b++)
    for (bl=freelist[b]; bl; bl=bl->next)
      CHECK1(bl);
#endif
}
#else
#define CHECK(p)
#endif

int times_malloc, times_free, times_realloc;
int used_slop, used_bblk, used_sbrk, times_split, times_s2b;
dump_stats()
{
  fprintf(stderr, "   malloc: %12d\n", times_malloc);
  fprintf(stderr, "     free: %12d\n", times_free);
  fprintf(stderr, "  realloc: %12d\n", times_realloc);
  fprintf(stderr, "used slop: %12d\n", used_slop);
  fprintf(stderr, "used bblk: %12d\n", used_bblk);
  fprintf(stderr, "used sbrk: %12d\n", used_sbrk);
  fprintf(stderr, "    split: %12d\n", times_split);
  fprintf(stderr, "      s2b: %12d\n", times_s2b);
}

static inline int
size2bucket(unsigned size)
{
  int rv=0;
  times_s2b++;
  size>>=2;
  while (size)
  {
    rv++;
    size>>=1;
  }
  return rv;
}

static inline BLOCK *
split_block(BLOCK *b, int size)
{
  BLOCK *rv = (BLOCK *)((char *)b + size+8);
  times_split++;
#if DEBUG
  printf("  split %d/%08x to %d/%08x, %d/%08x\n",
	 b->size, b, size, b, b->size - size - 8, rv);
#endif
  rv->size = b->size - size - 8;
  b->size = size;
  ENDSZ(b) = b->size;
  ENDSZ(rv) = rv->size;
  CHECK(b);
  CHECK(rv);
  return rv;
}

#define RET(rv) CHECK(rv); ENDSZ(rv) |= 1; rv->size |= 1; return DATA(rv)

char *
test_malloc(int size)
{
  int b, chunk_size;
  BLOCK *rv;
  static BLOCK *expected_sbrk = 0;

  static int initted=0;
  if (!initted)
  {
    initted = sbrk(0);
    atexit(dump_stats);
  }

  times_malloc++;

  if (size<4) size = 4;
  size = (size+3)&~3;
#if DEBUG
  printf("malloc(%d)\n", size);
#endif

  if (slop && slop->size >= size)
  {
    used_slop++;
    rv = slop;
#if DEBUG
    printf("  using slop %d/%08x\n", slop->size, slop);
#endif
    if (slop->size >= size+MIN_SAVE_EXTRA)
    {
      slop = split_block(slop, size);
#if DEBUG
      printf("  remaining slop %d/%08x\n", slop->size, slop);
#endif
    }
    else
      slop = 0;
    RET(rv);
  }

  if (sbrk_block && sbrk_block->size >= size)
  {
    used_bblk++;
    rv = sbrk_block;
#if DEBUG
    printf("  using sbrk_block %d/%08x\n", sbrk_block->size, sbrk_block);
#endif
    if (sbrk_block->size >= size+MIN_SAVE_EXTRA)
    {
      sbrk_block = split_block(sbrk_block, size);
#if DEBUG
      printf("  remaining sbrk_block %d/%08x\n", sbrk_block->size, sbrk_block);
#endif
    }
    else
      sbrk_block = 0;
    RET(rv);
  }

  b = size2bucket(size);
  while (b < 30)
  {
    BLOCK **prev = &(freelist[b]);
#if DEBUG
    printf("  checking bucket %d\n", b);
#endif
    for (rv=freelist[b]; rv; prev=&(rv->next), rv=rv->next)
      if (rv->size >= size)
      {
#if DEBUG
	printf("    found size %d/%08x\n", rv->size, rv);
#endif
	*prev = rv->next;
	if (rv->size >= size+MIN_SAVE_EXTRA)
	{
#if DEBUG
	  printf("    enough to save\n");
#endif
	  if (slop)
	  {
	    b = size2bucket(slop->size);
#if DEBUG
	    printf("    putting old slop %d/%08x on free list %d\n",
		   slop->size, slop, b);
#endif
	    slop->next = freelist[b];
	    freelist[b] = slop;
	  }
	  slop = split_block(rv, size);
#if DEBUG
	  printf("    slop size %d/%08x\n", slop->size, slop);
#endif
	}
	RET(rv);
      }
    b++;
  }

  if (sbrk_block)
  {
    b = size2bucket(sbrk_block->size);
#if DEBUG
    printf("  putting old sbrk_block %d/%08x on free list %d\n",
	   sbrk_block->size, sbrk_block, b);
#endif
    sbrk_block->next = freelist[b];
    freelist[b] = sbrk_block;
    sbrk_block = 0;
  }

  chunk_size = (size+16+SBRK_CHUNK-1)&~(SBRK_CHUNK-1);
  used_sbrk++;
  rv = (BLOCK *)sbrk(chunk_size);
  if (rv == 0)
    return 0;
#if DEBUG
  printf("sbrk(%d) -> %08x, expected %08x\n", chunk_size, rv, expected_sbrk);
#endif
  if (rv == expected_sbrk)
  {
    expected_sbrk = (BLOCK *)((char *)rv + chunk_size);
    /* absorb old end-block-marker */
#if DEBUG
    printf("  got expected sbrk\n");
#endif
    rv = (BLOCK *)((char *)rv - 4);
    if (sbrk_block && rv == AFTER(sbrk_block))
    {
#if DEBUG
      printf("    linking sbrk_block\n");
#endif
      sbrk_block->size += chunk_size;
      rv = sbrk_block;
    }
  }
  else
  {
    expected_sbrk = (BLOCK *)((char *)rv + chunk_size);
#if DEBUG
    printf("    disconnected sbrk\n");
#endif
    /* build start-block-marker */
    rv->size = 1;
    rv = (BLOCK *)((char *)rv + 4);
    chunk_size -= 8;
  }
  rv->size = chunk_size - 8;
  ENDSZ(rv) = rv->size;
  AFTER(rv)->size = 1;
  CHECK(rv);

  if (chunk_size >= size+MIN_SAVE_EXTRA)
  {
    CHECK(rv);
    sbrk_block = split_block(rv, size);
#if DEBUG
    printf("  new sbrk_block %d/%08x\n", sbrk_block->size, sbrk_block);
#endif
    CHECK(rv);
    CHECK(sbrk_block);
  }
  else
    sbrk_block = 0;

#if 0
  printf("\t\t\t\t\tsbrk %08x\n", (char *)rv + chunk_size - initted);
  for (b=0; b<30; b++)
  {
    BLOCK *bl;
    for (bl=freelist[b]; bl; bl=bl->next)
      printf("\t\t\t\t\t\t%d %12d\n", b, bl->size);
  }
#endif

  RET(rv);
}

static BLOCK *
merge(BLOCK *a, BLOCK *b, BLOCK *c)
{
  int bu;
  BLOCK *bp, **bpp;

#if DEBUG
  printf("  merge %d/%08x + %d/%08x = %d\n",
	 a->size, a, b->size, b, a->size+b->size+8);
#endif

  CHECK(a);
  CHECK(b);
  CHECK(c);
  if (c == sbrk_block)
  {
#if DEBUG
    printf("  snipping sbrk_block %d/%08x\n", sbrk_block->size, sbrk_block);
#endif
    sbrk_block = 0;
  }
  if (c == slop)
  {
#if DEBUG
    printf("  snipping slop %d/%08x\n", slop->size, slop);
#endif
    slop = 0;
  }
  bu = size2bucket(c->size);
#if DEBUG
  printf("bucket for %d/%08x is %d\n", c->size, c, bu);
#endif
  bpp = freelist+bu;
  for (bp=freelist[bu]; bp; bpp=&(bp->next), bp=bp->next)
  {
#if DEBUG
    printf("  %08x", bp);
#endif
    if (bp == c)
    {
#if DEBUG
      printf("\n  snipping %d/%08x from freelist[%d]\n", bp->size, bp, bu);
#endif
      *bpp = bp->next;
      break;
    }
  }
  CHECK(c);

  a->size += b->size + 8;
  ENDSZ(a) = a->size;

  CHECK(a);
  return a;
}

void
test_free(char *ptr)
{
  int b;
  BLOCK *block = (BLOCK *)(ptr-4);
  block->size &= ~1;
  ENDSZ(block) &= ~1;
  times_free++;
#if DEBUG
  printf("free(%d/%08x)\n", block->size, block);
#endif

  CHECK(block);
  if (! (AFTER(block)->size & 1))
  {
    CHECK(AFTER(block));
  }
  if (! (BEFSZ(block) & 1))
  {
    CHECK(BEFORE(block));
    block = merge(BEFORE(block), block, BEFORE(block));
  }
  CHECK(block);
  if (! (AFTER(block)->size & 1))
  {
    CHECK(AFTER(block));
    block = merge(block, AFTER(block), AFTER(block));
  }
  CHECK(block);
  
  b = size2bucket(block->size);
  block->next = freelist[b];
  freelist[b] = block;
  CHECK(block);
}

char *
test_realloc(char *ptr, int size)
{
  BLOCK *b = (BLOCK *)(ptr-4);
  char *newptr;
  int copysize = b->size;
  times_realloc++;
  if (size <= b->size)
  {
#if 0
    if (b->size < 2*MIN_SAVE_EXTRA
	|| (size >= b->size-512 && size >= b->size/2))
#endif
      return ptr;
    copysize = size;
  }

  newptr = (char *)test_malloc(size);
#if DEBUG
  printf("realloc %d %d/%08x %08x->%08, %d\n",
	 size, b->size, b, ptr, newptr, copysize);
#endif
  memcpy(newptr, ptr, copysize);
  test_free(ptr);
  return newptr;
}
