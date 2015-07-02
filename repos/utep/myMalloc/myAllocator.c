#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include "myAllocator.h"

/*
  This is a simple endogenous first-fit allocator.  

  Each allocated memory region is sandwiched between a "BlockPrefix"
  and a "BlockSuffix".  All block info is stored in its BlockPrefix.
  A block b's BlockSuffix is used by b's successor block to determine
  the address of b's prefix.  Prefixes' & Suffixes' sizes are rounded
  up to the next multiple of 8 bytes (see prefixSize, suffixSize,
  align8()).  Therefore a block must be at least of size
  prefixSize+suffixSize.  The method makeFreeBlock() fills in a prefix
  & suffix within a region, and marks it as free (->allocated=0).
  Such a block can be marked as allcated by setting its "allocated"
  field.  The usable space between a block's prefix &
  suffix (extent - (prefixSize+suffixSize) is computed by
  usableSpace().  

  All blocks are allocated from an arena extending from arenaBegin to
  arenaEnd.  In particular, the first block's prefix is at address
  arenaBegin, and the last block's suffix is at address
  arenaEnd-suffixSize. 

  This allocator generally refers to a block by the address of its
  prefix.  The address of the prefix to block b's successor is the
  address of b's suffix + suffixSize, and the address of block b's
  predecessor's suffix is the address of b's prefix -
  suffixSize.   See computeNextPrefixAddress(),
  computePrevSuffixAddr(), getNextPrefix(), getPrefPrefix().

  The method findFirstFit() searches the arena for a sufficiently
  large free block.  Adjacent free blocks can be coalesced:  See
  coalescePrev(),   coalesce().  

  Functions regionToBlock() and blockToRegion() convert between
  prefixes & the first available address within the block.

  FindFirstAllocRegion() uses findFirstFit to locate a suffiently
  large unallocated bock.  This block will be split if it contains
  sufficient excess space to create another free block.  FreeRegion
  marks the region's allocated block as free and attempts to coalesce
  it with its neighbors.

 */

/* block prefix & suffix */
typedef struct BlockPrefix_s {
  struct BlockSuffix_s *suffix;
  int allocated;
} BlockPrefix_t;

typedef struct BlockSuffix_s {
  struct BlockPrefix_s *prefix;
} BlockSuffix_t;

/* align everything to multiples of 8 */
#define align8(x) ((x+7) & ~7)
#define prefixSize align8(sizeof(BlockPrefix_t))
#define suffixSize align8(sizeof(BlockSuffix_t))

/* how much memory to ask for */
const size_t DEFAULT_BRKSIZE = 0x100000;	/* 1M */

/* create a block, mark it as free */
BlockPrefix_t *makeFreeBlock(void *addr, size_t size) { 
  BlockPrefix_t *p = addr;
  void *limitAddr = addr + size;
  BlockSuffix_t *s = limitAddr - align8(sizeof(BlockSuffix_t));
  p->suffix = s;
  s->prefix = p;
  p->allocated = 0;
  return p;
}

/* lowest & highest address in arena (global vars) */
BlockPrefix_t *arenaBegin = (void *)0;
void *arenaEnd = 0;

void initializeArena() {
    if (arenaBegin != 0)	/* only initialize once */
	return; 
    arenaBegin = makeFreeBlock(sbrk(DEFAULT_BRKSIZE), DEFAULT_BRKSIZE);
    arenaEnd = ((void *)arenaBegin) + DEFAULT_BRKSIZE;
}

size_t computeUsableSpace(BlockPrefix_t *p) { /* useful space within a block */
    void *prefix_end = ((void*)p) + prefixSize;
    return ((void *)(p->suffix)) - (prefix_end);
}

BlockPrefix_t *computeNextPrefixAddr(BlockPrefix_t *p) { 
    return ((void *)(p->suffix)) + suffixSize;
}

BlockSuffix_t *computePrevSuffixAddr(BlockPrefix_t *p) {
    return ((void *)p) - suffixSize;
}

BlockPrefix_t *getNextPrefix(BlockPrefix_t *p) { /* return addr of next block (prefix), or 0 if last */
    BlockPrefix_t *np = computeNextPrefixAddr(p);
    if ((void*)np < (void *)arenaEnd)
	return np;
    else
	return (BlockPrefix_t *)0;
}

BlockPrefix_t *getPrevPrefix(BlockPrefix_t *p) { /* return addr of prev block, or 0 if first */
    BlockSuffix_t *ps = computePrevSuffixAddr(p);
    if ((void *)ps > (void *)arenaBegin)
	return ps->prefix;
    else
	return (BlockPrefix_t *)0;
}

BlockPrefix_t *coalescePrev(BlockPrefix_t *p) {	/* coalesce p with prev, return prev if coalesced, otherwise p */
    BlockPrefix_t *prev = getPrevPrefix(p);
    if (p && prev && (!p->allocated) && (!prev->allocated)) {
	makeFreeBlock(prev, ((void *)computeNextPrefixAddr(p)) - (void *)prev);
	return prev;
    }
    return p;
}    


void coalesce(BlockPrefix_t *p) {	/* coalesce p with prev & next */
    if (p != (void *)0) {
        BlockPrefix_t *next;
	p = coalescePrev(p);
	next = getNextPrefix(p);
	if (next) 
	    coalescePrev(next);
    }
}

int growingDisabled = 1;	/* true: don't grow arena! (needed for cygwin) */

BlockPrefix_t *growArena(size_t s) { /* this won't work under cygwin since runtime uses brk()!! */
    void *n;
    BlockPrefix_t *p;
    if (growingDisabled)
	return (BlockPrefix_t *)0;
    s += (prefixSize + suffixSize);
    if (s < DEFAULT_BRKSIZE)
	s = DEFAULT_BRKSIZE;
    n = sbrk(s);
    if ((n == 0) || (n != arenaEnd)) /* fail if brk moved or failed! */
	return 0;
    arenaEnd = n + s;		/* new end */
    p = makeFreeBlock(n, s);	/* create new block */
    p = coalescePrev(p);	/* coalesce with old arena end  */
    return p;
}


int pcheck(void *p) {		/* check that pointer is within arena */
    return (p >= (void *)arenaBegin && p < (void *)arenaEnd);
}


void arenaCheck() {		/* consistency check */
    BlockPrefix_t *p = arenaBegin;
    size_t amtFree = 0, amtAllocated = 0;
    int numBlocks = 0;
    while (p != 0) {		/* walk through arena */
	fprintf(stderr, "  checking from 0x%llx, size=%lld, allocated=%d...\n",
		(long long)p,
		(long long)computeUsableSpace(p), p->allocated);
	assert(pcheck(p));	/* p must remain within arena */
	assert(pcheck(p->suffix)); /* suffix must be within arena */
	assert(p->suffix->prefix == p);	/* suffix should reference prefix */
	if (p->allocated) 	/* update allocated & free space */
	    amtAllocated += computeUsableSpace(p);
	else
	    amtFree += computeUsableSpace(p);
	numBlocks += 1;
	p = computeNextPrefixAddr(p);
	if (p == arenaEnd) {
	    break;
	} else {
	    assert(pcheck(p));
	}
    }
    fprintf(stderr,
	    " mcheck: numBlocks=%d, amtAllocated=%lldk, amtFree=%lldk, arenaSize=%lldk\n",
	    numBlocks,
	    (long long)amtAllocated / 1024LL,
	    (long long)amtFree/1024LL,
	    ((long long)arenaEnd - (long long)arenaBegin) / 1024LL);
}

BlockPrefix_t *findFirstFit(size_t s) {	/* find first block with usable space > s */
    BlockPrefix_t *p = arenaBegin;
    while (p) {
	if (!p->allocated && computeUsableSpace(p) >= s)
	    return p;
	p = getNextPrefix(p);
    }
    return growArena(s);
}

/* conversion between blocks & regions (offset of prefixSize */

BlockPrefix_t *regionToPrefix(void *r) {
  if (r)
    return r - prefixSize;
  else
    return 0;
}


void *prefixToRegion(BlockPrefix_t *p) {
  void * vp = p;
  if (p)
    return vp + prefixSize;
  else
    return 0;
}

/* these really are equivalent to malloc & free */

void *firstFitAllocRegion(size_t s) {
  size_t asize = align8(s);
  BlockPrefix_t *p;
  if (arenaBegin == 0)		/* arena uninitialized? */
    initializeArena();
  p = findFirstFit(s);		/* find a block */
  if (p) {			/* found a block */
    size_t availSize = computeUsableSpace(p);
    if (availSize >= (asize + prefixSize + suffixSize + 8)) { /* split block? */
      void *freeSliverStart = (void *)p + prefixSize + suffixSize + asize;
      void *freeSliverEnd = computeNextPrefixAddr(p);
      makeFreeBlock(freeSliverStart, freeSliverEnd - freeSliverStart);
      makeFreeBlock(p, freeSliverStart - (void *)p); /* piece being allocated */
    }
    p->allocated = 1;		/* mark as allocated */
    return prefixToRegion(p);	/* convert to *region */
  } else {			/* failed */
    return (void *)0;
  }
  
}

void freeRegion(void *r) {
    if (r != 0) {
	BlockPrefix_t *p = regionToPrefix(r); /* convert to block */
	p->allocated = 0;	/* mark as free */
	coalesce(p);
    }
}


/*
  like realloc(r, newSize), resizeRegion will return a new region of size
   newSize containing the old contents of r by:
   1. checking if the present region has sufficient available space to
   satisfy the request (if so, do nothing)
   2. allocating a new region of sufficient size & copying the data
   TODO: if the successor 's' to r's block is free, and there is sufficient space
   in r + s, then just adjust sizes of r & s.
*/
void *resizeRegion(void *r, size_t newSize) {
  int oldSize;
  if (r != (void *)0)		/* old region existed */
    oldSize = computeUsableSpace(regionToPrefix(r));
  else
    oldSize = 0;		/* non-existant regions have size 0 */
  if (oldSize >= newSize)	/* old region is big enough */
    return r;
  else {			/* allocate new region & copy old data */
    char *o = (char *)r;	/* treat both regions as char* */
    char *n = (char *)firstFitAllocRegion(newSize); 
    int i;
    for (i = 0; i < oldSize; i++) /* copy byte-by-byte, should use memcpy */
      n[i] = o[i];
    freeRegion(o);		/* free old region */
    return (void *)n;
  }
}

