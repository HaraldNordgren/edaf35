/* Several headers builds a circular linked list of free blocks.
 * The list of free blocks is sorted in memory order and the last header
 * points to the first to make the list circular.
 * The header is at least the size of the alignment type, which is a double.
 * If we are on a 64 bit system pointers are 64 bits and the header will be larger
 * to hold both the pointer and the size.
 * When memory is allocated the block is removed from the circular list but
 * is still used to keep track of the allocated blocks size.
 * The block's sizes are stored as the number of headers the block can hold including the header.
 *
 * First fit allocates memory in the first block of big enough size. The
 * Quickfit has a couple of standard sizes and makes allocations to them first.
 * If quickfit fails to find a quick fit in an appropriate size,
 * the memory will be allocated in the first fit order.
 * In rare cases there can be blocks which only can hold a header.
 * This happens when a block is chosen which is exactly one unit bigger than necessary
 *
 * If no block with big enough size is found, the boundary between stack and heap
 * is moved to increase the heap size.
 * The border is never moved back and memory is never given back to the operating system.
 */

#ifndef STRATEGY /* if not defined use system malloc */
#define STRATEGY 4 /* use quick fit as default */
#endif

#if STRATEGY != 0 /* 0 means use system malloc */

#include "malloc.h"

#include <string.h>
#include <unistd.h>

#if STRATEGY < 1 || STRATEGY > 4
#error STRATEGY must contain a value 1-4.
#endif

#define STRATEGY_FIRST 1
#define STRATEGY_BEST 2
#define STRATEGY_WORST 3
#define STRATEGY_QUICK 4

/* headers is used to create a circular linked list */
typedef union header
{
	struct {
		union header *next;
		size_t size;
	} s;
	double dummy; /* force alignment of blocks to the alignment of doubles */
} header;

/* base is a dummy header to avoid special cases when the list is empty */
header base = { .s = {&base, 0}};
header *freep = &base;

#define nalloc 1024 /* minimum amount of units to allocate each time, must be 2^x */

/* morecore increases the amount of heap space avalible */
header *morecore(size_t units)
{
	char *cp;
	header *hdr;

	if(units < nalloc)
		units = nalloc;

	cp = (char *) sbrk(units * sizeof(header));
	if(cp == (char *) -1)
		return NULL;
	hdr = (header *) cp;
	hdr->s.size = units;
	/* free((void *)(hdr + 1)); */ /* removed because quicklist's blocks have to be split and fixed outside */
	return hdr;
}

#if STRATEGY == STRATEGY_QUICK

#ifndef NRQUICKLISTS
#define NRQUICKLISTS 4
#endif

/* quicklist[] keeps track of headers to blocks of predefined sizes */
header *quicklist[NRQUICKLISTS] = {0};

#endif

void *malloc(size_t size)
{
#if STRATEGY == STRATEGY_WORST || STRATEGY == STRATEGY_BEST
	/* best and worst strategy uses some extra pointers when finding worst and best */
	header *best, *prevbest;
	size_t value;
#endif
	/* p the temporary iteration pointer, prevp is the previous value of p to make it easier to modify the list */
	header *p, *prevp;
	size_t units;

	/* if the user don't want any memory, we are done. */
	if(size == 0)
		return NULL;

	units = (size + sizeof(header)-1) / sizeof(header) + 1; /* get enough units, rounded up.*/

#if STRATEGY == STRATEGY_QUICK
	if(units <= (4 << NRQUICKLISTS)) /* is the number of units small enough to take from a quick list? */
	{
		int list;
		size_t listsize, i;
		for(list = 0; units > (8 << list); ++list); /* find which list this block should be taken from */

		if(quicklist[list] != NULL) /* if there are blocks, use them */
		{
			header* hdr = quicklist[list];
			quicklist[list] = hdr->s.next;
			return (void *) (hdr+1);
		}
		/* else make new ones */
		listsize = 8 << list; /* listsize is the size of each block in this list */

		/* determine the number of units to allocate */
		units = listsize;
		if(units < nalloc)
			units = nalloc;

		/* push the border to increase the heap limit */
		header *more = morecore(units);

		if(more == NULL)
			return NULL;

		for(i = 0; i < units; i += listsize)
		{
			/* split block and put in lists */
			more[i].s.size = listsize;
			free((void *) &more[i+1]);
		}

		/* unlink one block from the list to return */
		header* hdr = quicklist[list];
		quicklist[list] = hdr->s.next;
		return (void *) (hdr+1);
	}
#endif

#if STRATEGY == STRATEGY_WORST
	best = NULL;
	value = 0;
#elif STRATEGY == STRATEGY_BEST
	best = NULL;
	value = (size_t) -1;
#endif

	/* loop through the circular list */
	prevp = freep;
	for(p = prevp->s.next;; prevp = p, p = p->s.next)
	{
		/* test if this block is big enough */
		if(p->s.size >= units)
		{
#if STRATEGY == STRATEGY_WORST
			/* keep track of worst */
			if(p->s.size > value)
			{
				best = p;
				prevbest = prevp;
				value = p->s.size;
			}
#elif STRATEGY == STRATEGY_BEST
			/* keep track of best */
			if(p->s.size <= value)
			{
				best = p;
				prevbest = prevp;
				value = p->s.size;
			}
#else
			/* use this block */
			if(p->s.size == units)
				/* unlink nothing left to store */
				prevp->s.next = p->s.next;
			else
			{
				/* allocate tail, keep present block */
				p->s.size -= units;
				p += p->s.size;
				p->s.size = units;
			}
			freep = prevp;
			return (void *)(p+1);
#endif
		}
		if(p == freep) /* if we come back to start */
		{
#if STRATEGY == STRATEGY_WORST || STRATEGY == STRATEGY_BEST
			if(best == NULL) /* no available block */
			{
				/* push border */
				best = morecore(units);
				if(best == NULL)
					return NULL;
				free((void *)(best+1));
				/* find the new block and also find the previous block */
				for(prevp = &base, p = base.s.next; p->s.size < units; prevp = p, p = p->s.next);
				/* set these new headers as prevbest and best to avoid code duplication */
				prevbest = prevp;
				best = p;
			}
			/* block to be used is best (or worst) */
			p = best;
			prevp = prevbest;
			if(p->s.size == units)
				/* unlink nothing left to store */
				prevp->s.next = p->s.next;
			else
			{
				/* allocate tail, keep present block */
				p->s.size -= units;
				p += p->s.size;
				p->s.size = units;
			}
			freep = prevp;
			return (void *)(p+1);
#else
			/* if no block with sufficient size was found, create a new one */
			if((p = morecore(units)) == NULL)
				return NULL;
			free((void *)(p + 1));
#endif
		}
	}
}


/* This realloc implementation was designed to be very simple.
 * A few optimizations could've been made: merging blocks before
 * determining if a copy has to be made or not or reducing the size of
 * the block by breaking off a chunk of the block if size is smaller
 * than already allocated. */
void *realloc(void *ptr, size_t size)
{
	header *hdr;
	size_t old_size;
	void *new_ptr;

	/* basic cases with NULL and size == 0 */
	if(ptr == NULL)
		return malloc(size);

	if(size == 0)
	{
		free(ptr);
		return NULL;
	}

	/* allocate another memory area, copy data, then free the old area */
	hdr = ((header *)ptr) - 1;
	old_size = (hdr->s.size-1) * sizeof(header);
	if(old_size > size)
		old_size = size;

	new_ptr = malloc(size);
	if(new_ptr == NULL)
		return NULL;

	memcpy(new_ptr, ptr, old_size);
	free(ptr);

	return new_ptr;
}

/* free inserts a block of data into the list (or lists) of avalible blocks */
void free(void *ptr)
{
	header *bp, *p;
	if(ptr == NULL)
		return;

	bp = (header *)ptr - 1;    /* point to block header */

#if STRATEGY == STRATEGY_QUICK
	/* is the number of units small enough to have been taken from a quick list? */
	if(bp->s.size <= (4 << NRQUICKLISTS))
	{
		size_t list;
		header *x;
		int count = 0;
		/* find which list this block was taken from */
		for(list = 0; bp->s.size > (8 << list); ++list);

		bp->s.next = quicklist[list]; /* put it back */
		quicklist[list] = bp;
		return;
	}
#endif

	/* K&R code. */
	/* break if bp is inbetween p and p->s.next or is outside of p->s.next and p when the list loops back */
	for (p = freep; !(bp > p && bp < p->s.next); p = p->s.next)
		if (p >= p->s.next && (bp > p || bp < p->s.next))
			break; /* freed block at start or end of arena */

	if (bp + bp->s.size == p->s.next) {    /* join to upper nbr */
		bp->s.size += p->s.next->s.size;
		bp->s.next = p->s.next->s.next;
	} else
		bp->s.next = p->s.next;
	if (p + p->s.size == bp) {            /* join to lower nbr */
		p->s.size += bp->s.size;
		p->s.next = bp->s.next;
	} else
		p->s.next = bp;

	freep = p;
}

#endif
